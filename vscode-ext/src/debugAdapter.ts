import {
    Logger, logger,
    LoggingDebugSession,
    InitializedEvent, TerminatedEvent, StoppedEvent, BreakpointEvent, OutputEvent,
    ProgressStartEvent, ProgressUpdateEvent, ProgressEndEvent,
    InvalidatedEvent, Thread, StackFrame, Scope, Source, Handles, Breakpoint,
    DebugSession, Event
} from '@vscode/debugadapter';
import { DebugProtocol } from '@vscode/debugprotocol';
import * as fs from 'fs';
import * as path from 'path';

// Import our Emscripten WASM module
const createMemu8086 = require('../wasm/build/memu8086_core.js');

interface LaunchRequestArguments extends DebugProtocol.LaunchRequestArguments {
    program: string;
    stopOnEntry?: boolean;
}

export class MemuDebugSession extends LoggingDebugSession {
    private static threadID = 1;
    private _emu: any = null;
    private _sourceFile: string = "";
    private _dashboardMemoryBase: number | null = null;

    public constructor() {
        super("memu8086-debug.txt");
        this.setDebuggerLinesStartAt1(true);
        this.setDebuggerColumnsStartAt1(true);
    }

    protected initializeRequest(response: DebugProtocol.InitializeResponse, args: DebugProtocol.InitializeRequestArguments): void {
        response.body = response.body || {};
        response.body.supportsConfigurationDoneRequest = true;
        response.body.supportsEvaluateForHovers = true;
        response.body.supportsStepBack = false;
        response.body.supportsSetVariable = false;
        response.body.supportsReadMemoryRequest = true;
        
        this.sendResponse(response);
        this.sendEvent(new InitializedEvent());
    }

    protected configurationDoneRequest(response: DebugProtocol.ConfigurationDoneResponse, args: DebugProtocol.ConfigurationDoneArguments): void {
        super.configurationDoneRequest(response, args);
    }

    protected async launchRequest(response: DebugProtocol.LaunchResponse, args: LaunchRequestArguments) {
        this._sourceFile = args.program;
        
        try {
            const Module = await createMemu8086({
                // Node.js doesn't need locateFile for WASM in the same dir typically, but just in case:
                locateFile: function(pathName: string) {
                    if (pathName.endsWith('.wasm')) return path.join(__dirname, '../wasm/build/memu8086_core.wasm');
                    return pathName;
                }
            });
            
            this._emu = new Module.WasmEmulator();
            
            const sourceCode = fs.readFileSync(this._sourceFile, 'utf8');
            this._emu.reset();
            const success = this._emu.assemble(sourceCode);
            
            if (!success) {
                const errs = this._emu.get_errors();
                this.sendEvent(new OutputEvent(`Assembly failed!\n${errs}`, 'stderr'));
                this.sendResponse(response);
                this.sendEvent(new TerminatedEvent());
                return;
            }

            this.sendResponse(response);
            
            this.sendUpdateDashboardEvent();
            
            if (args.stopOnEntry) {
                this.sendEvent(new StoppedEvent('entry', MemuDebugSession.threadID));
            } else {
                this.continueRequest(<DebugProtocol.ContinueResponse>{}, { threadId: MemuDebugSession.threadID });
            }

        } catch (e: any) {
            this.sendEvent(new OutputEvent(`Failed to load WASM emulator: ${e.message}\n`, 'stderr'));
            this.sendResponse(response);
            this.sendEvent(new TerminatedEvent());
        }
    }

    private sendUpdateDashboardEvent() {
        if (this._emu) {
            const regs = this._emu.get_registers();
            
            const ss_offset = regs.SS * 16;
            const stackAddr = ss_offset + regs.SP;
            const stackView = this._emu.get_memory(stackAddr, 64);
            
            let ds_offset = regs.DS * 16;
            if (this._dashboardMemoryBase !== null) {
                ds_offset = this._dashboardMemoryBase;
            }
            const memView = this._emu.get_memory(ds_offset, 256);

            const symbols = this._emu.get_symbols();
            const variables = [];
            for (let i = 0; i < symbols.length; i++) {
                const sym = symbols[i];
                const addr = (regs.DS * 16) + sym.offset;
                const valView = this._emu.get_memory(addr, sym.size);
                
                let valStr = "0x";
                // little endian
                for (let j = valView.length - 1; j >= 0; j--) {
                    valStr += valView[j].toString(16).padStart(2, '0').toUpperCase();
                }
                
                variables.push({
                    name: sym.name,
                    address: "0x" + addr.toString(16).padStart(5, '0').toUpperCase(),
                    value: valStr,
                    size: sym.size
                });
            }

            this.sendEvent(new Event('updateDashboard', {
                regs: regs,
                line: this._emu.get_current_line(),
                state: this._emu.is_halted() ? "HALTED" : "PAUSED",
                stackData: Buffer.from(stackView).toString('hex'),
                stackAddr: stackAddr,
                memoryData: Buffer.from(memView).toString('hex'),
                memoryAddr: ds_offset,
                variables: variables
            }));
        }
    }

    protected nextRequest(response: DebugProtocol.NextResponse, args: DebugProtocol.NextArguments): void {
        if (this._emu) {
            this._emu.step();
            const out = this._emu.get_output();
            if (out) this.sendEvent(new OutputEvent(out, 'console'));
        }
        this.sendResponse(response);
        this.sendUpdateDashboardEvent();
        this.sendEvent(new StoppedEvent('step', MemuDebugSession.threadID));
    }

    protected stepInRequest(response: DebugProtocol.StepInResponse, args: DebugProtocol.StepInArguments): void {
        this.nextRequest(response, args); // Same as next for now
    }

    protected continueRequest(response: DebugProtocol.ContinueResponse, args: DebugProtocol.ContinueArguments): void {
        this.sendResponse(response);
        
        if (this._emu) {
            this._emu.run(); // Calls our updated WasmEmulator::run() which stops on breakpoints
            const out = this._emu.get_output();
            if (out) this.sendEvent(new OutputEvent(out, 'console'));
            
            this.sendUpdateDashboardEvent();
            if (this._emu.is_halted()) {
                this.sendEvent(new TerminatedEvent());
            } else {
                this.sendEvent(new StoppedEvent('breakpoint', MemuDebugSession.threadID));
            }
        } else {
            this.sendEvent(new TerminatedEvent());
        }
    }

    protected threadsRequest(response: DebugProtocol.ThreadsResponse): void {
        response.body = {
            threads: [
                new Thread(MemuDebugSession.threadID, "8086 Core")
            ]
        };
        this.sendResponse(response);
    }

    protected stackTraceRequest(response: DebugProtocol.StackTraceResponse, args: DebugProtocol.StackTraceArguments): void {
        const line = this._emu ? this._emu.get_current_line() : 1;
        const src = new Source(path.basename(this._sourceFile), this._sourceFile);
        response.body = {
            stackFrames: [
                new StackFrame(1, "main", src, line, 1)
            ],
            totalFrames: 1
        };
        this.sendResponse(response);
    }

    protected scopesRequest(response: DebugProtocol.ScopesResponse, args: DebugProtocol.ScopesArguments): void {
        response.body = {
            scopes: [
                new Scope("General Registers", 1, false),
                new Scope("Pointer & Index", 2, false),
                new Scope("Segment Registers", 3, false),
                new Scope("Flags", 4, false)
            ]
        };
        this.sendResponse(response);
    }

    protected variablesRequest(response: DebugProtocol.VariablesResponse, args: DebugProtocol.VariablesArguments): void {
        const variables: DebugProtocol.Variable[] = [];
        
        if (this._emu) {
            const regs = this._emu.get_registers();
            let keys: string[] = [];
            
            if (args.variablesReference === 1) {
                keys = ['AX', 'BX', 'CX', 'DX'];
            } else if (args.variablesReference === 2) {
                keys = ['SP', 'BP', 'SI', 'DI', 'IP'];
            } else if (args.variablesReference === 3) {
                keys = ['CS', 'DS', 'ES', 'SS'];
            } else if (args.variablesReference === 4) {
                const flags = regs.flags;
                for (const f of ['CF', 'PF', 'AF', 'ZF', 'SF', 'TF', 'IF', 'DF', 'OF']) {
                    variables.push({
                        name: f,
                        type: "boolean",
                        value: flags[f] ? "1" : "0",
                        variablesReference: 0
                    });
                }
            }

            if (args.variablesReference >= 1 && args.variablesReference <= 3) {
                for (const r of keys) {
                    let memRef: string | undefined = undefined;
                    // Segment registers represent a 16-byte shifted address
                    if (args.variablesReference === 3) {
                        memRef = (regs[r] * 16).toString();
                    } 
                    // Pointers (SP, BP, SI, DI, IP) can represent memory offsets within a segment
                    // For simplicity, let's associate SP/BP with SS, SI/DI with DS, IP with CS
                    else if (args.variablesReference === 2) {
                        let seg = regs.DS;
                        if (r === 'SP' || r === 'BP') seg = regs.SS;
                        if (r === 'IP') seg = regs.CS;
                        memRef = ((seg * 16) + regs[r]).toString();
                    }

                    variables.push({
                        name: r,
                        type: "uint16",
                        value: "0x" + regs[r].toString(16).padStart(4, '0').toUpperCase(),
                        variablesReference: 0,
                        memoryReference: memRef
                    });
                }
            }
        }
        
        response.body = { variables };
        this.sendResponse(response);
    }

    protected setBreakPointsRequest(response: DebugProtocol.SetBreakpointsResponse, args: DebugProtocol.SetBreakpointsArguments): void {
        const path = args.source.path;
        const clientLines = args.breakpoints || [];
        
        const linesToSet = clientLines.map(bp => bp.line);
        let actualBreakpoints: DebugProtocol.Breakpoint[] = [];
        
        if (this._emu && path === this._sourceFile) {
            const verifiedBps = this._emu.set_breakpoints(linesToSet);
            for (let i = 0; i < verifiedBps.length; i++) {
                const bpInfo = verifiedBps[i];
                actualBreakpoints.push(new Breakpoint(bpInfo.verified, bpInfo.line));
            }
        } else {
            actualBreakpoints = clientLines.map(bp => new Breakpoint(false, bp.line));
        }
        
        response.body = {
            breakpoints: actualBreakpoints
        };
        this.sendResponse(response);
    }

    protected readMemoryRequest(response: DebugProtocol.ReadMemoryResponse, args: DebugProtocol.ReadMemoryArguments): void {
        if (this._emu) {
            const memoryReference = parseInt(args.memoryReference);
            const count = args.count;
            const memView = this._emu.get_memory(memoryReference, count);
            response.body = {
                address: args.memoryReference,
                unreadableBytes: count - memView.length,
                data: Buffer.from(memView).toString('base64')
            };
        } else {
            response.body = {
                address: args.memoryReference,
                unreadableBytes: args.count,
                data: ""
            };
        }
        this.sendResponse(response);
    }
    protected customRequest(command: string, response: DebugProtocol.Response, args: any, request?: DebugProtocol.Request): void {
        if (command === 'setDashboardMemory') {
            this._dashboardMemoryBase = parseInt(args.address);
            this.sendUpdateDashboardEvent();
            this.sendResponse(response);
            return;
        }
        super.customRequest(command, response, args, request);
    }
}

DebugSession.run(MemuDebugSession);
