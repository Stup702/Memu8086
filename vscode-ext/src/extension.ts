import * as vscode from 'vscode';
import * as path from 'path';

export function activate(context: vscode.ExtensionContext) {
    console.log('MEMU8086 extension is now active!');

    const provider = new MemuDashboardProvider(context.extensionUri);
    
    context.subscriptions.push(
        vscode.window.registerWebviewViewProvider('memu8086.dashboardView', provider, {
            webviewOptions: { retainContextWhenHidden: true }
        })
    );

    const highlightDecoration = vscode.window.createTextEditorDecorationType({
        backgroundColor: 'rgba(255, 255, 0, 0.3)',
        isWholeLine: true
    });

    // Provide the provider with the highlight function
    provider.setHighlightCallback((line: number) => {
        const editor = vscode.window.activeTextEditor;
        if (editor && line >= 1) { 
            const vsCodeLine = line - 1;
            if (vsCodeLine < editor.document.lineCount) {
                const range = editor.document.lineAt(vsCodeLine).range;
                editor.setDecorations(highlightDecoration, [range]);
                editor.revealRange(range, vscode.TextEditorRevealType.InCenterIfOutsideViewport);
            }
        } else if (editor) {
            editor.setDecorations(highlightDecoration, []);
        }
    });

    const writeEmitter = new vscode.EventEmitter<string>();
    const pty: vscode.Pseudoterminal = {
        onDidWrite: writeEmitter.event,
        open: () => {},
        close: () => {},
        handleInput: data => {
            // User typed something into the terminal. Send it to WASM!
            for (let i = 0; i < data.length; i++) {
                provider.sendAction('console_input', data.charCodeAt(i));
            }
        }
    };
    const terminal = vscode.window.createTerminal({ name: 'MEMU8086 Console', pty });

    // Provide the provider with the terminal write callback
    provider.setTerminalCallback((text: string) => {
        writeEmitter.fire(text);
        terminal.show(true); // show but don't steal focus
    });

    const sendAction = (action: string) => {
        const editor = vscode.window.activeTextEditor;
        if (!editor) {
            vscode.window.showErrorMessage('No active editor found!');
            return;
        }
        provider.sendAction(action, editor.document.getText());
    };

    context.subscriptions.push(
        vscode.commands.registerCommand('memu8086.chippingIn', () => {
            vscode.commands.executeCommand('memu8086.dashboardView.focus');
        }),
        vscode.commands.registerCommand('memu8086.run', () => sendAction('run')),
        vscode.commands.registerCommand('memu8086.step', () => sendAction('step')),
        vscode.commands.registerCommand('memu8086.reset', () => {
            provider.sendAction('reset', '');
            const editor = vscode.window.activeTextEditor;
            if (editor) editor.setDecorations(highlightDecoration, []);
        })
    );
}

class MemuDashboardProvider implements vscode.WebviewViewProvider {
    private _view?: vscode.WebviewView;
    private highlightCallback?: (line: number) => void;
    private terminalCallback?: (text: string) => void;

    constructor(private readonly _extensionUri: vscode.Uri) { }

    setHighlightCallback(cb: (line: number) => void) {
        this.highlightCallback = cb;
    }
    
    setTerminalCallback(cb: (text: string) => void) {
        this.terminalCallback = cb;
    }

    public resolveWebviewView(
        webviewView: vscode.WebviewView,
        context: vscode.WebviewViewResolveContext,
        _token: vscode.CancellationToken,
    ) {
        this._view = webviewView;
        webviewView.webview.options = { enableScripts: true };

        const wasmJsUri = webviewView.webview.asWebviewUri(vscode.Uri.joinPath(this._extensionUri, 'wasm', 'build', 'memu8086_core.js'));
        const wasmBinUri = webviewView.webview.asWebviewUri(vscode.Uri.joinPath(this._extensionUri, 'wasm', 'build', 'memu8086_core.wasm'));

        webviewView.webview.html = this._getHtmlForWebview(wasmJsUri, wasmBinUri);

        webviewView.webview.onDidReceiveMessage(data => {
            if (data.command === 'highlightLine' && this.highlightCallback) {
                this.highlightCallback(data.line);
            } else if (data.command === 'console_output' && this.terminalCallback) {
                this.terminalCallback(data.text);
            }
        });
    }

    public sendAction(action: string, source: any) {
        if (this._view) {
            this._view.webview.postMessage({ command: action, source: source });
        } else {
            vscode.commands.executeCommand('memu8086.dashboardView.focus').then(() => {
                setTimeout(() => {
                    this._view?.webview.postMessage({ command: action, source: source });
                }, 500);
            });
        }
    }

    private _getHtmlForWebview(wasmJsUri: vscode.Uri, wasmBinUri: vscode.Uri) {
        return `<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <style>
        body { font-family: sans-serif; padding: 10px; color: var(--vscode-editor-foreground); }
        h3 { margin: 10px 0 5px 0; color: var(--vscode-textLink-foreground); font-size: 12px; text-transform: uppercase;}
        
        .regs-grid { display: grid; grid-template-columns: repeat(2, 1fr); gap: 4px; margin-bottom: 15px; }
        .reg-box { background: var(--vscode-editor-inactiveSelectionBackground); padding: 4px; border-radius: 3px; font-family: monospace; font-size: 13px;}
        .reg-name { font-weight: bold; color: #4fc1ff; display: inline-block; width: 25px;}
        
        .flags-grid { display: grid; grid-template-columns: repeat(3, 1fr); gap: 4px; }
        .flag-box { padding: 3px; border-radius: 3px; font-family: monospace; font-size: 11px; font-weight: bold; text-align: center; }
        .flag-on { background: #1f6b3e; color: #fff; }
        .flag-off { background: var(--vscode-editor-inactiveSelectionBackground); color: var(--vscode-descriptionForeground); }
        
        .status-bar { margin-bottom: 15px; padding: 6px; border-radius: 3px; font-weight: bold; font-size: 11px; text-align: center;}
        .status-idle { background: var(--vscode-editor-inactiveSelectionBackground); }
        .status-running { background: #0e639c; color: white;}
        .status-halted { background: #8a2b2b; color: white;}
    </style>
</head>
<body>
    <div class="status-bar status-idle" id="statusBox">State: IDLE</div>
    <h3>Registers</h3>
    <div class="regs-grid" id="regsContainer"><div class="reg-box">Loading...</div></div>
    <h3>Flags</h3>
    <div class="flags-grid" id="flagsContainer"></div>

    <script src="${wasmJsUri.toString()}"></script>
    <script>
        const vscode = acquireVsCodeApi();
        let emu;
        let lastAsmSource = "";
        let isLoaded = false;
        
        createMemu8086({
            locateFile: function(path) {
                if (path.endsWith('.wasm')) return '${wasmBinUri.toString()}';
                return path;
            }
        }).then(function(Module) {
            emu = new Module.WasmEmulator();
            updateUI();
        });

        window.addEventListener('message', event => {
            if (!emu) return;
            const msg = event.data;
            
            if (msg.command === 'console_input') {
                emu.send_input(msg.source);
                return;
            }
            
            if (msg.command === 'run' || msg.command === 'step') {
                if (msg.source !== lastAsmSource || !isLoaded) {
                    emu.reset();
                    const success = emu.assemble(msg.source);
                    if (!success) {
                        setStatus("ERROR: Assembly Failed", "status-halted");
                        vscode.postMessage({ command: 'highlightLine', line: -1 });
                        return;
                    }
                    lastAsmSource = msg.source;
                    isLoaded = true;
                    // If we just loaded the code, we highlight line 1 but DO NOT execute it yet!
                    // This creates the proper "Start Debugging" experience.
                    updateUI();
                    
                    // If the command was run, we still want to run it completely.
                    if (msg.command === 'step') return; 
                }
                
                if (msg.command === 'run') {
                    // Step in a loop until halted or error to avoid freezing the tab completely
                    let safeCount = 0;
                    while (!emu.is_halted() && safeCount < 100000) {
                        emu.step();
                        
                        // Push any IO output during the run loop!
                        const out = emu.get_output();
                        if (out.length > 0) vscode.postMessage({ command: 'console_output', text: out });
                        
                        safeCount++;
                    }
                } else if (msg.command === 'step') {
                    emu.step();
                    const out = emu.get_output();
                    if (out.length > 0) vscode.postMessage({ command: 'console_output', text: out });
                }
                updateUI();
            } else if (msg.command === 'reset') {
                emu.reset();
                lastAsmSource = "";
                isLoaded = false;
                vscode.postMessage({ command: 'highlightLine', line: -1 });
                updateUI();
            }
        });

        function updateUI() {
            if (!emu) return;
            const regs = emu.get_registers();
            
            const container = document.getElementById('regsContainer');
            container.innerHTML = '';
            const regNames = ['AX', 'BX', 'CX', 'DX', 'SI', 'DI', 'SP', 'BP', 'IP', 'CS', 'DS', 'ES', 'SS'];
            for (const r of regNames) {
                const val = regs[r].toString(16).padStart(4, '0').toUpperCase();
                container.innerHTML += \`<div class="reg-box"><span class="reg-name">\${r}</span> 0x\${val}</div>\`;
            }
            
            const flagsContainer = document.getElementById('flagsContainer');
            flagsContainer.innerHTML = '';
            const flags = regs.flags;
            const flagNames = ['CF', 'PF', 'AF', 'ZF', 'SF', 'TF', 'IF', 'DF', 'OF'];
            for (const f of flagNames) {
                const isOn = flags[f];
                flagsContainer.innerHTML += \`<div class="flag-box \${isOn ? 'flag-on' : 'flag-off'}">\${f}:\${isOn ? '1' : '0'}</div>\`;
            }
            
            if (emu.is_halted()) {
                setStatus("HALTED", "status-halted");
                vscode.postMessage({ command: 'highlightLine', line: -1 });
            } else {
                setStatus(isLoaded ? "READY" : "IDLE", isLoaded ? "status-running" : "status-idle");
                vscode.postMessage({ command: 'highlightLine', line: isLoaded ? emu.get_current_line() : -1 });
            }
        }
        
        function setStatus(text, cssClass) {
            const box = document.getElementById('statusBox');
            box.innerText = "State: " + text;
            box.className = "status-bar " + cssClass;
        }
    </script>
</body>
</html>`;
    }
}
export function deactivate() {}
