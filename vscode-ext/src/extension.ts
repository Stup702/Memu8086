import * as vscode from 'vscode';

export function activate(context: vscode.ExtensionContext) {
    console.log('MEMU8086 extension is now active in DAP mode!');

    const provider = new MemuDashboardProvider(context.extensionUri);
    
    context.subscriptions.push(
        vscode.window.registerWebviewViewProvider('memu8086.dashboardView', provider, {
            webviewOptions: { retainContextWhenHidden: true }
        })
    );

    context.subscriptions.push(
        vscode.debug.onDidReceiveDebugSessionCustomEvent(e => {
            if (e.event === 'updateDashboard') {
                provider.updateData(e.body.regs, e.body.state, e.body.line, e.body.stackData, e.body.stackAddr, e.body.memoryData, e.body.memoryAddr, e.body.variables);
                // Force focus back to our dashboard when VS Code tries to steal it for the native debug view
                setTimeout(() => {
                    vscode.commands.executeCommand('memu8086.dashboardView.focus');
                }, 100);
            }
        })
    );

    context.subscriptions.push(
        vscode.commands.registerCommand('memu8086.chippingIn', () => {
            const editor = vscode.window.activeTextEditor;
            if (!editor) {
                vscode.window.showErrorMessage('No active editor found to debug!');
                return;
            }

            if (vscode.debug.activeDebugSession && vscode.debug.activeDebugSession.type === 'memu8086') {
                vscode.window.showWarningMessage('A Memu8086 debug session is already running!');
                vscode.commands.executeCommand('memu8086.dashboardView.focus');
                return;
            }

            vscode.commands.executeCommand('memu8086.dashboardView.focus');
            
            provider.openInspector();

            vscode.debug.startDebugging(undefined, {
                type: 'memu8086',
                name: 'Debug DOS',
                request: 'launch',
                program: editor.document.uri.fsPath,
                stopOnEntry: true
            });
        })
    );
}

export class MemuDashboardProvider implements vscode.WebviewViewProvider {
    private _view?: vscode.WebviewView;
    private _inspectorPanel?: vscode.WebviewPanel;
    private _lastData?: any;
    
    private _showStack: boolean = true;
    private _showMemory: boolean = true;

    constructor(private readonly _extensionUri: vscode.Uri) { }

    resolveWebviewView(webviewView: vscode.WebviewView) {
        this._view = webviewView;
        webviewView.webview.options = { enableScripts: true };
        webviewView.webview.html = this._getHtml();
        
        webviewView.webview.onDidReceiveMessage(async msg => {
            if (msg.command === 'toggleStack') {
                this._showStack = !this._showStack;
                if (!this._inspectorPanel && this._showStack) {
                    this.openInspector(false);
                }
                this.broadcastToggles();
            }
            if (msg.command === 'toggleMemory') {
                this._showMemory = !this._showMemory;
                if (!this._inspectorPanel && this._showMemory) {
                    this.openInspector(false);
                }
                this.broadcastToggles();
            }
            if (msg.command === 'viewMemory') {
                if (vscode.debug.activeDebugSession) {
                    vscode.debug.activeDebugSession.customRequest('setDashboardMemory', { address: msg.address });
                }
                this._showMemory = true;
                this.broadcastToggles();
                this.openInspector(false);
            }
        });
    }

    private broadcastToggles() {
        const payload = {
            command: 'updateToggles',
            showStack: this._showStack,
            showMemory: this._showMemory
        };
        if (this._view) {
            this._view.webview.postMessage(payload);
        }
        if (this._inspectorPanel) {
            this._inspectorPanel.webview.postMessage(payload);
        }
    }

    public updateData(regs: any, state: string, line: number, stackData: string, stackAddr: number, memoryData: string, memoryAddr: number, variables: any[]) {
        this._lastData = { regs, state, line, stackData, stackAddr, memoryData, memoryAddr, variables };
        if (this._view) {
            this._view.webview.postMessage({ command: 'updateUI', ...this._lastData });
        }
        if (this._inspectorPanel) {
            this._inspectorPanel.webview.postMessage({ command: 'updateUI', ...this._lastData });
        }
    }

    public openInspector(takeFocus: boolean = true) {
        if (!this._inspectorPanel) {
            this._inspectorPanel = vscode.window.createWebviewPanel(
                'memu8086.inspector',
                'Memory & Stack',
                vscode.ViewColumn.Beside,
                { enableScripts: true }
            );
            this._inspectorPanel.onDidDispose(() => {
                this._inspectorPanel = undefined;
            });
            this._inspectorPanel.webview.html = this._getInspectorHtml();
        } else if (takeFocus) {
            this._inspectorPanel.reveal(vscode.ViewColumn.Beside, !takeFocus);
        }
        
        if (this._lastData) {
            this._inspectorPanel.webview.postMessage({ command: 'updateUI', ...this._lastData });
        }
    }

    private _getHtml() {
        return `<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <style>
        body { font-family: sans-serif; padding: 10px; color: var(--vscode-editor-foreground); }
        h3 { margin: 15px 0 5px 0; color: var(--vscode-textLink-foreground); font-size: 12px; text-transform: uppercase;}
        
        .regs-grid { display: grid; grid-template-columns: repeat(2, 1fr); gap: 4px; margin-bottom: 5px; }
        .reg-box { background: var(--vscode-editor-inactiveSelectionBackground); padding: 4px; border-radius: 3px; font-family: monospace; font-size: 13px;}
        .reg-name { font-weight: bold; color: #4fc1ff; display: inline-block; width: 25px;}
        .section-label { font-size: 10px; color: var(--vscode-descriptionForeground); margin-bottom: 3px; margin-top: 10px; }
        
        .flags-grid { display: grid; grid-template-columns: repeat(3, 1fr); gap: 4px; margin-bottom: 15px;}
        .flag-box { padding: 3px; border-radius: 3px; font-family: monospace; font-size: 11px; font-weight: bold; text-align: center; }
        .flag-on { background: #1f6b3e; color: #fff; }
        .flag-off { background: var(--vscode-editor-inactiveSelectionBackground); color: var(--vscode-descriptionForeground); }
        h3 { margin: 0 0 10px 0; color: var(--vscode-textLink-foreground); font-size: 14px; text-transform: uppercase;}
        
        .btn-row { display: flex; gap: 10px; margin-bottom: 20px; }
        .btn { 
            flex: 1; padding: 8px; border: none; cursor: pointer; color: white;
            border-radius: 4px; font-weight: bold; text-align: center;
        }
        .btn-active { background-color: var(--vscode-button-background); }
        .btn-inactive { background-color: var(--vscode-button-secondaryBackground); color: var(--vscode-button-secondaryForeground); }
        .btn:hover { opacity: 0.8; }
        
        .section-label { font-size: 11px; text-transform: uppercase; margin-bottom: 4px; color: var(--vscode-descriptionForeground); margin-top: 10px;}
        .regs-grid { display: grid; grid-template-columns: 1fr 1fr; gap: 4px; }
        .reg-box { background: var(--vscode-editor-inactiveSelectionBackground); padding: 4px; border-radius: 3px; font-family: monospace; font-size: 13px; display: flex; justify-content: space-between; }
        .reg-name { color: var(--vscode-symbolIcon-variableForeground); font-weight: bold; }
        
        .flags-grid { display: grid; grid-template-columns: repeat(3, 1fr); gap: 4px; }
        .flag-box { padding: 4px; text-align: center; font-family: monospace; font-size: 12px; border-radius: 3px; }
        .flag-on { background: var(--vscode-testing-iconPassed); color: var(--vscode-editor-background); }
        .flag-off { background: var(--vscode-testing-iconFailed); color: var(--vscode-editor-background); opacity: 0.6; }
        
        .status-bar { padding: 8px; font-weight: bold; text-align: center; border-radius: 4px; margin-bottom: 15px;}
        .status-running { background: var(--vscode-button-background); color: var(--vscode-button-foreground); }
        .status-halted { background: var(--vscode-errorForeground); color: var(--vscode-editor-background); }
        
        table { width: 100%; border-collapse: collapse; margin-top: 5px; font-family: monospace; font-size: 13px; }
        th, td { text-align: left; padding: 4px; border-bottom: 1px solid var(--vscode-editorGroup-border); }
        th { color: var(--vscode-descriptionForeground); font-size: 11px; text-transform: uppercase; }
        
    </style>
</head>
<body>
    <h3>MEMU8086: DASHBOARD</h3>
    <div id="statusBox" class="status-bar status-running">State: WAITING</div>
    
    <div class="btn-row">
        <button id="btnToggleStack" class="btn ${this._showStack ? 'btn-active' : 'btn-inactive'}" onclick="vscode.postMessage({command:'toggleStack'})">
            Toggle Stack
        </button>
        <button id="btnToggleMemory" class="btn ${this._showMemory ? 'btn-active' : 'btn-inactive'}" onclick="vscode.postMessage({command:'toggleMemory'})">
            Toggle Memory
        </button>
    </div>

    <h3>REGISTERS</h3>
    <div id="regsContainer"></div>

    <h3 style="margin-top:20px;">FLAGS</h3>
    <div class="flags-grid" id="flagsContainer"></div>

    <h3 style="margin-top:20px;">VARIABLES</h3>
    <table>
        <thead><tr><th>Name</th><th>Addr</th><th>Value</th></tr></thead>
        <tbody id="varsBody"></tbody>
    </table>

    <script>
        const vscode = acquireVsCodeApi();

        function viewMemory(address) {
            vscode.postMessage({ command: 'viewMemory', address: address });
        }

        let lastRegs = null;
        let lastFlags = null;

        window.addEventListener('message', event => {
            const msg = event.data;
            if (msg.command === 'updateUI') {
                updateUI(msg);
            } else if (msg.command === 'updateToggles') {
                const btnStack = document.getElementById('btnToggleStack');
                if (btnStack) btnStack.className = msg.showStack ? 'btn btn-active' : 'btn btn-inactive';
                const btnMem = document.getElementById('btnToggleMemory');
                if (btnMem) btnMem.className = msg.showMemory ? 'btn btn-active' : 'btn btn-inactive';
            }
        });

        function updateUI(msg) {
            const regs = msg.regs;
            const container = document.getElementById('regsContainer');
            container.innerHTML = '';
            
            const groups = [
                { name: "General", keys: ['AX', 'BX', 'CX', 'DX'] },
                { name: "Pointer & Index", keys: ['SP', 'BP', 'SI', 'DI', 'IP'] },
                { name: "Segment", keys: ['CS', 'DS', 'ES', 'SS'] }
            ];
            
            for (const g of groups) {
                container.innerHTML += \`<div class="section-label">\${g.name}</div>\`;
                let grid = '<div class="regs-grid">';
                for (const r of g.keys) {
                    const val = regs[r].toString(16).padStart(4, '0').toUpperCase();
                    let highlight = '';
                    if (lastRegs && lastRegs[r] !== regs[r]) {
                        highlight = 'color: #d7ba7d; text-shadow: 0 0 5px rgba(215,186,125,0.5);';
                    }
                    const tooltip = \`Unsigned: \${regs[r]} \\nSigned: \${regs[r] > 32767 ? regs[r] - 65536 : regs[r]} \\nBinary: \${regs[r].toString(2).padStart(16, '0')}\`;
                    
                    if (g.name === "Segment") {
                        const addr = regs[r] * 16;
                        grid += \`<div class="reg-box" style="cursor:pointer" onclick="viewMemory(\${addr})" title="\${tooltip}"><span class="reg-name">\${r}</span> <span style="\${highlight}">0x\${val}</span></div>\`;
                    } else {
                        grid += \`<div class="reg-box" title="\${tooltip}"><span class="reg-name">\${r}</span> <span style="\${highlight}">0x\${val}</span></div>\`;
                    }
                }
                grid += '</div>';
                container.innerHTML += grid;
            }
            
            const flagsContainer = document.getElementById('flagsContainer');
            flagsContainer.innerHTML = '';
            const flags = regs.flags;
            const flagNames = ['CF', 'PF', 'AF', 'ZF', 'SF', 'TF', 'IF', 'DF', 'OF'];
            for (const f of flagNames) {
                const isOn = flags[f];
                let flagClass = isOn ? 'flag-on' : 'flag-off';
                let flagStyle = '';
                if (lastFlags && lastFlags[f] !== isOn) {
                    flagStyle = 'box-shadow: 0 0 5px #d7ba7d; border: 1px solid #d7ba7d;';
                }
                flagsContainer.innerHTML += \`<div class="flag-box \${flagClass}" style="\${flagStyle}">\${f}:\${isOn ? '1' : '0'}</div>\`;
            }
            
            const box = document.getElementById('statusBox');
            box.innerText = "State: " + msg.state;
            if (msg.state === "HALTED" || msg.state === "ERROR") {
                box.className = "status-bar status-halted";
            } else {
                box.className = "status-bar status-running";
            }
            
            const varsBody = document.getElementById('varsBody');
            varsBody.innerHTML = '';
            if (msg.variables && msg.variables.length > 0) {
                for (const v of msg.variables) {
                    const intVal = parseInt(v.value, 16);
                    let signed = intVal;
                    const maxUnsigned = Math.pow(2, v.size * 8) - 1;
                    const maxSigned = Math.pow(2, (v.size * 8) - 1) - 1;
                    if (intVal > maxSigned) signed = intVal - (maxUnsigned + 1);
                    const bin = intVal.toString(2).padStart(v.size * 8, '0');
                    const tooltip = \`Unsigned: \${intVal} \\nSigned: \${signed} \\nBinary: \${bin}\`;
                    
                    let highlight = '';
                    if (lastRegs && lastRegs[\`_var_\${v.name}\`] !== v.value) {
                        highlight = 'color: #d7ba7d; text-shadow: 0 0 5px rgba(215,186,125,0.5);';
                    }
                    regs[\`_var_\${v.name}\`] = v.value; // Stash it in regs for next comparison

                    varsBody.innerHTML += \`<tr style="cursor:pointer" onclick="viewMemory('\${v.address}')" title="\${tooltip}">
                        <td style="color:var(--vscode-symbolIcon-variableForeground)">\${v.name}</td>
                        <td style="color:var(--vscode-descriptionForeground)">\${v.address}</td>
                        <td style="\${highlight}">\${v.value}</td>
                    </tr>\`;
                }
            } else {
                varsBody.innerHTML = '<tr><td colspan="3" style="text-align:center;color:var(--vscode-descriptionForeground)">No variables</td></tr>';
            }
            
            lastRegs = {...regs};
            lastFlags = {...flags};
        }
    </script>
</body>
</html>`;
    }




    private _getInspectorHtml() {
        return `<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <style>
        body, html { margin: 0; padding: 0; height: 100%; font-family: sans-serif; color: var(--vscode-editor-foreground); display: flex; flex-direction: column; background: var(--vscode-editor-background); }
        h3 { margin: 0; padding: 8px; background: var(--vscode-editorGroupHeader-tabsBackground); border-bottom: 1px solid var(--vscode-editorGroup-border); color: var(--vscode-tab-activeForeground); font-size: 11px; text-transform: uppercase; font-weight: normal; letter-spacing: 1px; text-align: center; }
        
        .panel { flex: 1; display: flex; flex-direction: column; overflow: hidden; min-height: 0; border-bottom: 2px solid var(--vscode-editorGroup-border); }
        .panel-content { flex: 1; overflow-y: auto; padding: 10px; }
        
        .mem-grid { display: grid; gap: 4px; font-family: monospace; font-size: 13px;}
        .mem-cell { background: var(--vscode-editor-inactiveSelectionBackground); padding: 4px; text-align: center; border-radius: 3px; }
        .mem-addr { background: transparent; color: var(--vscode-symbolIcon-variableForeground); font-weight: bold; text-align: right; padding-right: 10px;}
        
    </style>
</head>
<body>
    <div class="panel" id="stackPanel" style="display: ${this._showStack ? 'flex' : 'none'}">
        <h3>STACK</h3>
        <div class="panel-content">
            <div class="mem-grid" id="stackGrid" style="grid-template-columns: 80px repeat(8, 1fr);">Waiting for Debugger...</div>
        </div>
    </div>

    <div class="panel" id="memoryPanel" style="display: ${this._showMemory ? 'flex' : 'none'}">
        <h3>MEMORY</h3>
        <div class="panel-content">
            <div class="mem-grid" id="memoryGrid" style="grid-template-columns: 80px repeat(16, 1fr);">Waiting for Debugger...</div>
        </div>
    </div>

    <div id="emptyMsg" style="display: ${(!this._showStack && !this._showMemory) ? 'block' : 'none'}; padding: 20px; text-align: center; color: var(--vscode-descriptionForeground);">
        Please toggle Stack or Memory from the Dashboard to view.
    </div>

    <script>
        window.addEventListener('message', event => {
            const msg = event.data;
            if (msg.command === 'updateUI') {
                if (msg.stackData) renderHex('stackGrid', msg.stackData, msg.stackAddr, 8);
                if (msg.memoryData) renderHex('memoryGrid', msg.memoryData, msg.memoryAddr, 16);
            } else if (msg.command === 'updateToggles') {
                document.getElementById('stackPanel').style.display = msg.showStack ? 'flex' : 'none';
                document.getElementById('memoryPanel').style.display = msg.showMemory ? 'flex' : 'none';
                document.getElementById('emptyMsg').style.display = (!msg.showStack && !msg.showMemory) ? 'block' : 'none';
            }
        });

        function renderHex(gridId, hexStr, startAddr, cols) {
            const grid = document.getElementById(gridId);
            if (!grid) return;
            grid.innerHTML = '';
            
            const bytes = [];
            for (let i = 0; i < hexStr.length; i += 2) {
                bytes.push(hexStr.substr(i, 2).toUpperCase());
            }
            
            for (let i = 0; i < bytes.length; i += cols) {
                const addr = (startAddr + i).toString(16).padStart(5, '0').toUpperCase();
                grid.innerHTML += \`<div class="mem-cell mem-addr">0x\${addr}</div>\`;
                
                for (let j = 0; j < cols; j++) {
                    if (i + j < bytes.length) {
                        grid.innerHTML += \`<div class="mem-cell">\${bytes[i + j]}</div>\`;
                    } else {
                        grid.innerHTML += \`<div class="mem-cell" style="opacity: 0.2">--</div>\`;
                    }
                }
            }
        }
    </script>
</body>
</html>`;
    }
}

export function deactivate() {}
