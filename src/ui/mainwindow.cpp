#include "mainwindow.h"
#include "editor_panel.h"
#include "registers_panel.h"
#include "memory_panel.h"
#include "stack_panel.h"
#include "variables_panel.h"
#include "console_panel_widget.h"
#include "toolbar.h"
#include "theme.h"
#include "settings_dialog.h"
#include "examples_dialog.h"

#include <QVBoxLayout>
#include <QGridLayout>
#include <QPushButton>
#include <QMenuBar>
#include <QStatusBar>
#include <QSettings>
#include <QFileDialog>
#include <QMessageBox>
#include <QFile>
#include <QTextStream>
#include <QAction>
#include <QKeySequence>
#include <QFileInfo>
#include <QWindow>
#include <QTimer>
#include <QThread>

namespace memu8086::ui {

class DockContentWrapper : public QWidget {
public:
    DockContentWrapper(QWidget* content, QDockWidget* dock, bool enable_redock_btn = true) : QWidget(dock) {
        QVBoxLayout* layout = new QVBoxLayout(this);
        layout->setContentsMargins(0, 0, 0, 0);
        layout->setSpacing(0);
        layout->addWidget(content);

        if (enable_redock_btn) {
            QPushButton* btn_redock = new QPushButton("⊞ Redock to Main Window", this);
            btn_redock->setCursor(Qt::PointingHandCursor);
            btn_redock->setStyleSheet(
                QString("QPushButton { background-color: %1; color: %2; border: none; border-radius: 0px; padding: 8px; font-weight: bold; }"
                "QPushButton:hover { background-color: %3; }"
                "QPushButton:pressed { background-color: %4; }").arg(Theme::Color::ACCENT, Theme::Color::BG, Theme::Color::ACCENT_HOVER, Theme::Color::ACCENT_PRESSED)
            );
            layout->addWidget(btn_redock);
            btn_redock->hide(); // Hide initially while docked

            connect(dock, &QDockWidget::topLevelChanged, btn_redock, &QWidget::setVisible);
            connect(btn_redock, &QPushButton::clicked, dock, [dock]() {
                dock->setFloating(false);
                
                // Re-apply default layout sizes after the window has successfully re-docked
                if (auto* mainWin = qobject_cast<QMainWindow*>(dock->window())) {
                    QTimer::singleShot(50, mainWin, [mainWin]() {
                        auto docks = mainWin->findChildren<QDockWidget*>();
                        QDockWidget *de = nullptr, *dv = nullptr, *dr = nullptr, *ds = nullptr, *dm = nullptr;
                        for (auto* d : docks) {
                            if (d->objectName() == "dock_editor") de = d;
                            else if (d->objectName() == "dock_variables") dv = d;
                            else if (d->objectName() == "dock_registers") dr = d;
                            else if (d->objectName() == "dock_stack") ds = d;
                            else if (d->objectName() == "dock_memory") dm = d;
                        }
                        if (dr && de) mainWin->resizeDocks({dr, de}, {Theme::layout.reg_w, Theme::layout.ed_w}, Qt::Horizontal);
                        if (de && dv) mainWin->resizeDocks({de, dv}, {Theme::layout.var_w, Theme::layout.var_h}, Qt::Horizontal);
                        if (de && dm) mainWin->resizeDocks({de, dm}, {Theme::layout.ed_h, Theme::layout.mem_h}, Qt::Vertical);
                        if (dv && ds) mainWin->resizeDocks({dv, ds}, {Theme::layout.ed_h, Theme::layout.stack_h}, Qt::Vertical);
                    });
                }
            });
        }
    }
};

class TextTitleBar : public QWidget {
    QDockWidget* dock;

public:
    TextTitleBar(const QString& title, QDockWidget* dock) : QWidget(dock), dock(dock) {
        setObjectName("TextTitleBar");
        setAttribute(Qt::WA_StyledBackground, true);
        
        QHBoxLayout* layout = new QHBoxLayout(this);
        layout->setContentsMargins(10, 4, 6, 4);
        layout->setSpacing(8);

        QLabel* lbl = new QLabel(title.toUpper(), this);
        lbl->setStyleSheet("font-weight: bold; letter-spacing: 1px;");
        lbl->setAttribute(Qt::WA_TransparentForMouseEvents);

        QPushButton* btn_float = new QPushButton("Undock", this);
        QPushButton* btn_close = new QPushButton("Close", this);
        btn_float->setCursor(Qt::PointingHandCursor);
        btn_close->setCursor(Qt::PointingHandCursor);

        layout->addWidget(lbl);
        layout->addStretch();
        layout->addWidget(btn_float);
        layout->addWidget(btn_close);

        connect(btn_float, &QPushButton::clicked, dock, [dock]() { dock->setFloating(!dock->isFloating()); });
        connect(btn_close, &QPushButton::clicked, dock, &QDockWidget::close);

        connect(dock, &QDockWidget::topLevelChanged, this, [btn_float](bool floating) {
            btn_float->setVisible(!floating);
        });
    }

    QSize sizeHint() const override { return QSize(150, 40); }
    QSize minimumSizeHint() const override { return QSize(100, 40); }

protected:
    void mousePressEvent(QMouseEvent* event) override {
        if (event->button() == Qt::LeftButton && dock->isFloating()) {
            if (QWindow* window = dock->windowHandle()) {
                window->startSystemMove();
                event->accept();
                return;
            }
        }
        QWidget::mousePressEvent(event);
    }
    
    void mouseDoubleClickEvent(QMouseEvent* event) override {
        dock->setFloating(!dock->isFloating());
        QWidget::mouseDoubleClickEvent(event);
    }
};

MainWindow::MainWindow(emu8086::core::CPU& cpu, emu8086::assembler::Assembler& asm_,
                       emu8086::core::Debugger& dbg, 
                       emu8086::core::ConsoleState& console,
                       QWidget* parent)
    : QMainWindow(parent), cpu(cpu), asm_(asm_), dbg(dbg), console(console)
{
    Theme::generate_template();
    QString saved_theme = QSettings("memu8086", "memu8086").value("theme_name", "Dark").toString();
    Theme::apply_theme(saved_theme);
    
    // Initialize panels
    editor = new EditorPanel(this);
    registers_panel = new RegistersPanel(cpu, this);
    memory_panel = new MemoryPanel(this);
    stack_panel = new StackPanel(cpu, this);
    variables_panel = new VariablesPanel(this);
    console_panel = new ConsolePanelWidget(console, this);
    toolbar_widget = new Toolbar(this);

    setup_dock_panels();
    setup_toolbar();
    setup_menu_bar();
    setup_status_bar();
    setup_shortcuts();

    // Breakpoint wiring
    connect(editor, &EditorPanel::breakpoint_toggled, [this](int line) {
        // Convert source line → IP address using debug symbol info (approximate: use line as index)
        // For now toggle by line number heuristic
        uint16_t addr = line; // placeholder: replace with real line→addr mapping
        if (this->dbg.has_breakpoint(addr)) this->dbg.remove_breakpoint(addr);
        else this->dbg.add_breakpoint(addr);
        
        std::set<int> bp_lines;
        for (auto bp : this->dbg.get_breakpoints()) bp_lines.insert(bp);
        editor->set_breakpoint_lines(bp_lines);
    });

    // Speed wiring
    connect(toolbar_widget, &Toolbar::speed_changed, [this](float ips) {
        this->dbg.set_speed(ips);
    });

    // Memory → jump wiring
    connect(variables_panel, &VariablesPanel::show_in_memory, [this](uint32_t addr) {
        memory_panel->jump_to(addr);
        dock_memory->show();
        dock_memory->raise();
    });

    connect(stack_panel, &StackPanel::jump_to_memory, [this](uint32_t addr) {
        memory_panel->jump_to(addr);
        dock_memory->raise();
    });

    // Console input wiring
    this->console.on_key = [](char) {
        // Called from interrupt handler — nothing extra needed,
        // state.input_buffer is read directly by interrupt.cpp
    };
    
    // Forward Qt key events from console panel to the state
    connect(console_panel, &ConsolePanelWidget::key_pressed, [this](char c) {
        this->dbg.send_key(c);
    });

    tick_timer = new QTimer(this);
    connect(tick_timer, &QTimer::timeout, this, &MainWindow::on_debugger_tick);
    tick_timer->start(16); // ~60fps

    // Restore persistence settings
    QSettings s("memu8086", "memu8086");
    
    // Maximize window FIRST so layout ratios calculate accurately against the full screen!
    setWindowState(Qt::WindowMaximized);

    if (s.value("layout_user_saved", false).toBool()) {
        if (s.contains("geometry")) restoreGeometry(s.value("geometry").toByteArray());
        if (s.contains("windowState")) restoreState(s.value("windowState").toByteArray());
    } else {
        reset_dock_layout();
        
        // Position the floating console window at the bottom right initially
        QTimer::singleShot(100, this, [this]() {
            if (dock_console->isFloating()) {
                QRect screen = this->geometry();
                int cx = qMax(screen.left() + 20, screen.right() - dock_console->width() - 40);
                int cy = qMax(screen.top() + 20, screen.bottom() - dock_console->height() - 40);
                dock_console->move(cx, cy);
                dock_console->raise();
            }
        });
    }

    // Ensure all docks are shown
    for (QDockWidget* d : {dock_editor, dock_variables, dock_registers,
                           dock_stack, dock_memory, dock_console}) {
        d->show();
    }

    current_file = s.value("lastFile").toString();

    // Apply theme at startup from settings

    update_ui_state();
    
    if (!current_file.isEmpty() && QFile::exists(current_file)) {
        on_load_example(current_file);
    } else {
        on_new_file();
    }
}

void MainWindow::setup_dock_panels() {
    // Disabled AllowTabbedDocks to prevent panels from merging into VS Code style tabs!
    setDockOptions(QMainWindow::AnimatedDocks | QMainWindow::AllowNestedDocks);

    dock_editor = new QDockWidget("Code Editor", this);
    dock_editor->setWidget(new DockContentWrapper(editor, dock_editor));
    dock_editor->setTitleBarWidget(new TextTitleBar("Code Editor", dock_editor));
    dock_editor->setObjectName("dock_editor");

    dock_variables = new QDockWidget("Variables", this);
    dock_variables->setWidget(new DockContentWrapper(variables_panel, dock_variables));
    dock_variables->setTitleBarWidget(new TextTitleBar("Variables", dock_variables));
    dock_variables->setObjectName("dock_variables");

    dock_registers = new QDockWidget("Registers", this);
    dock_registers->setWidget(new DockContentWrapper(registers_panel, dock_registers));
    dock_registers->setTitleBarWidget(new TextTitleBar("Registers", dock_registers));
    dock_registers->setObjectName("dock_registers");

    dock_stack = new QDockWidget("Stack", this);
    dock_stack->setWidget(new DockContentWrapper(stack_panel, dock_stack));
    dock_stack->setTitleBarWidget(new TextTitleBar("Stack", dock_stack));
    dock_stack->setObjectName("dock_stack");

    dock_memory = new QDockWidget("Memory", this);
    dock_memory->setWidget(new DockContentWrapper(memory_panel, dock_memory));
    dock_memory->setTitleBarWidget(new TextTitleBar("Memory", dock_memory));
    dock_memory->setObjectName("dock_memory");
    
    dock_console = new QDockWidget("Console", this);
    dock_console->setWidget(new DockContentWrapper(console_panel, dock_console, false));
    dock_console->setTitleBarWidget(new TextTitleBar("Console", dock_console));
    dock_console->setObjectName("dock_console");
}

void MainWindow::reset_dock_layout() {
    // Remove docks from areas to cleanly break any accidental tabs/splits
    for (QDockWidget* d : {dock_editor, dock_variables, dock_registers,
                           dock_stack, dock_memory, dock_console}) {
        removeDockWidget(d);
    }

    addDockWidget(Qt::LeftDockWidgetArea, dock_registers);
    
    splitDockWidget(dock_registers, dock_editor, Qt::Horizontal);
    splitDockWidget(dock_editor, dock_variables, Qt::Horizontal);
    splitDockWidget(dock_editor, dock_memory, Qt::Vertical);
    splitDockWidget(dock_variables, dock_stack, Qt::Vertical);

    addDockWidget(Qt::BottomDockWidgetArea, dock_console);
    dock_console->setFloating(true); // Undock by default to prevent clutter

    // Calling removeDockWidget() implicitly hides widgets, so we must show them again
    for (QDockWidget* d : {dock_editor, dock_variables, dock_registers,
                           dock_stack, dock_memory, dock_console}) {
        d->show();
    }

    // Sizes MUST be applied after widgets are shown and the layout is mathematically validated!
    QTimer::singleShot(50, this, [this]() {
        resizeDocks({dock_registers, dock_editor}, {Theme::layout.reg_w, Theme::layout.ed_w}, Qt::Horizontal);
        resizeDocks({dock_editor, dock_variables}, {Theme::layout.var_w, Theme::layout.var_h}, Qt::Horizontal);
        resizeDocks({dock_editor, dock_memory}, {Theme::layout.ed_h, Theme::layout.mem_h}, Qt::Vertical);
        resizeDocks({dock_variables, dock_stack}, {Theme::layout.ed_h, Theme::layout.stack_h}, Qt::Vertical);
    });
}

void MainWindow::setup_toolbar() {
    addToolBar(Qt::TopToolBarArea, toolbar_widget);
    connect(toolbar_widget, &Toolbar::assemble_clicked, this, &MainWindow::on_assemble);
    connect(toolbar_widget, &Toolbar::run_clicked, this, &MainWindow::on_run);
    connect(toolbar_widget, &Toolbar::stop_clicked, this, &MainWindow::on_stop);
    connect(toolbar_widget, &Toolbar::step_clicked, this, &MainWindow::on_step);
    connect(toolbar_widget, &Toolbar::step_over_clicked, this, &MainWindow::on_step_over);
    connect(toolbar_widget, &Toolbar::reset_clicked, this, &MainWindow::on_reset);
}

void MainWindow::setup_menu_bar() {
    QMenu* file_menu = menuBar()->addMenu("File");
    file_menu->addAction("New", QKeySequence::New, this, &MainWindow::on_new_file);
    file_menu->addAction("Open...", QKeySequence::Open, this, &MainWindow::on_open_file);
    file_menu->addAction("Save", QKeySequence::Save, this, &MainWindow::on_save_file);
    file_menu->addAction("Save As...", QKeySequence::SaveAs, this, &MainWindow::on_save_as);
    file_menu->addSeparator();
    
    recent_files_menu = file_menu->addMenu("Recent Files");
    update_recent_files_menu();
    
    file_menu->addSeparator();
    file_menu->addAction("Exit", this, &QWidget::close);

    QMenu* view_menu = menuBar()->addMenu("View");
    view_menu->addAction(dock_editor->toggleViewAction());
    view_menu->addAction(dock_variables->toggleViewAction());
    view_menu->addAction(dock_registers->toggleViewAction());
    view_menu->addAction(dock_stack->toggleViewAction());
    view_menu->addAction(dock_memory->toggleViewAction());
    view_menu->addAction(dock_console->toggleViewAction());
    view_menu->addSeparator();
    
    QAction* act_save_layout = view_menu->addAction("Save Current Layout");
    QAction* act_reset_layout = view_menu->addAction("Reset to Default Layout");

    connect(act_save_layout, &QAction::triggered, this, [this]() {
        QSettings s("memu8086", "memu8086");
        s.setValue("geometry", saveGeometry());
        s.setValue("windowState", saveState());
        s.setValue("layout_user_saved", true);
    });

    connect(act_reset_layout, &QAction::triggered, this, [this]() {
        reset_dock_layout();
        QSettings s("memu8086", "memu8086");
        s.setValue("layout_user_saved", false);
        s.remove("geometry");
        s.remove("windowState");
    });

    QMenu* emu_menu = menuBar()->addMenu("Emulator");
    emu_menu->addAction("Assemble", QKeySequence(Qt::Key_F5), this, &MainWindow::on_assemble);
    emu_menu->addAction("Run", QKeySequence(Qt::Key_F9), this, &MainWindow::on_run);
    emu_menu->addAction("Step", QKeySequence(Qt::Key_F7), this, &MainWindow::on_step);
    emu_menu->addAction("Step Over", QKeySequence(Qt::Key_F8), this, &MainWindow::on_step_over);
    emu_menu->addAction("Stop", QKeySequence(Qt::Key_Escape), this, &MainWindow::on_stop);
    emu_menu->addAction("Reset", this, &MainWindow::on_reset);

    QMenu* examples_menu = menuBar()->addMenu("Examples");
    examples_menu->addAction("Browse Examples...", this, &MainWindow::show_examples);

    QMenu* help_menu = menuBar()->addMenu("Help");
    help_menu->addAction("Settings", this, &MainWindow::show_settings);
    help_menu->addSeparator();
    help_menu->addAction("About memu8086", this, &MainWindow::show_about);
}

void MainWindow::setup_status_bar() {
    status_file = new QLabel("Untitled", this);
    statusBar()->addWidget(status_file);

    status_state = new QLabel(this);
    status_ip = new QLabel(this);
    status_cycles = new QLabel(this);

    statusBar()->addPermanentWidget(status_state);
    statusBar()->addPermanentWidget(status_ip);
    statusBar()->addPermanentWidget(status_cycles);
}

void MainWindow::setup_shortcuts() {
    // Additional shortcuts can be defined here if they aren't bound to menu actions
    // Shortcuts are defined in the menus
}

void MainWindow::update_recent_files_menu() {
    recent_files_menu->clear();
    QSettings settings("memu8086", "memu8086");
    QStringList recent_files = settings.value("recentFiles").toStringList();
    
    for (const QString& file : recent_files) {
        QAction* action = recent_files_menu->addAction(QFileInfo(file).fileName());
        connect(action, &QAction::triggered, this, [this, file]() {
            on_load_example(file); // Reusing load method for generic file paths
        });
    }
}

void MainWindow::on_debugger_tick() {
    static auto prev_state = DebuggerState::IDLE;
    if (dbg.get_state() == DebuggerState::RUNNING) {
        prev_snapshot = dbg.get_prev_snapshot();
        dbg.run_frame(0.016f);
        refresh_panels();
        update_ui_state();
    } else {
        dbg.run_frame(0.0f); // Drain any remaining IO
        if (prev_state == DebuggerState::RUNNING) {
            refresh_panels();
            update_ui_state();
        }
    }
    prev_state = dbg.get_state();
    // Always refresh console (it might have new output)
    console_panel->refresh();
}

void MainWindow::refresh_panels() {
    registers_panel->refresh();
    memory_panel->update(cpu.mem, cpu);
    stack_panel->update(cpu);
    variables_panel->update(cpu);
    

    // Update exec line in editor
    uint16_t ip = cpu.regs.IP;
    if (assembled && last_output.offset_to_line.count(ip)) {
        editor->set_exec_line(last_output.offset_to_line.at(ip));
    } else {
        editor->set_exec_line(-1);
    }

    status_ip->setText(QString("IP: %1").arg(ip, 4, 16, QChar('0')).toUpper());
    status_cycles->setText(QString("Cycles: %1").arg(dbg.get_cycle_count()));
}

void MainWindow::update_ui_state() {
    auto s = dbg.get_state();
    toolbar_widget->set_state(s, editor->is_modified(), assembled);

    auto set_status = [this](const QString& text, const QString& color) {
        status_state->setText(QString("<span style='color: %1; font-weight: bold;'>%2</span>").arg(color, text));
    };

    switch(s) {
        case DebuggerState::IDLE:    set_status("● Ready",    "#4A9EFF"); break;
        case DebuggerState::RUNNING: set_status("● Running…", "#4AFF8C"); break;
        case DebuggerState::PAUSED:  set_status("● Paused",   "#FFB347"); break;
        case DebuggerState::HALTED:  set_status("● Halted",   "#FFB347"); break;
        case DebuggerState::ERROR:
            set_status("✕ " + QString::fromStdString(dbg.get_last_error()), "#FF5A5A"); break;
    }
    
    QString file_display = current_file.isEmpty() ? "Untitled" : QFileInfo(current_file).fileName();
    if (editor->is_modified()) {
        status_file->setText(QString("<i>%1*</i>").arg(file_display));
    } else {
        status_file->setText(file_display);
    }
}

void MainWindow::on_assemble() {
    QString source = editor->get_source();
    last_output = asm_.assemble(source.toStdString(), 0x0100);
    editor->set_errors(last_output.errors);

    if (last_output.success) {
        dbg.load_program(last_output);
        variables_panel->set_symbols(last_output.symbols);
        assembled = true;
        status_state->setText("● Assembled OK");
        
        // Highlight the very first instruction immediately!
        if (last_output.offset_to_line.count(cpu.regs.IP)) {
            editor->set_exec_line(last_output.offset_to_line.at(cpu.regs.IP));
        }
    } else {
        assembled = false;
        // show error count in status
        int errs = static_cast<int>(last_output.errors.size());
        status_state->setText(QString("✕ %1 error(s)").arg(errs));
        status_state->setStyleSheet("color: #FF5A5A");
    }
    toolbar_widget->set_state(dbg.get_state(), editor->is_modified(), assembled);
}

void MainWindow::on_run() { 
    if (!assembled) { on_assemble(); if (!assembled) return; }
    if (dbg.get_state() == DebuggerState::RUNNING) dbg.stop();
    else dbg.run();
    update_ui_state();
}

void MainWindow::on_stop() { 
    dbg.stop(); 
    update_ui_state(); 
}

void MainWindow::on_step() { 
    if (!assembled) { on_assemble(); if (!assembled) return; }
    prev_snapshot = dbg.get_prev_snapshot();
    
    toolbar_widget->set_state(DebuggerState::RUNNING, editor->is_modified(), assembled);
    
    QThread* thread = QThread::create([this]() {
        dbg.step();
        QMetaObject::invokeMethod(this, [this]() {
            refresh_panels();
            update_ui_state();
        }, Qt::QueuedConnection);
    });
    connect(thread, &QThread::finished, thread, &QObject::deleteLater);
    thread->start();
}

void MainWindow::on_step_over() { 
    if (!assembled) { on_assemble(); if (!assembled) return; }
    prev_snapshot = dbg.get_prev_snapshot();
    
    toolbar_widget->set_state(DebuggerState::RUNNING, editor->is_modified(), assembled);
    
    QThread* thread = QThread::create([this]() {
        dbg.step_over();
        QMetaObject::invokeMethod(this, [this]() {
            refresh_panels();
            update_ui_state();
        }, Qt::QueuedConnection);
    });
    connect(thread, &QThread::finished, thread, &QObject::deleteLater);
    thread->start();
}

void MainWindow::on_reset() { 
    dbg.reset();
    console_panel->refresh();
    editor->set_exec_line(-1);
    assembled = false;
    update_ui_state();
    refresh_panels();
}

void MainWindow::on_new_file() {
    current_file.clear();
    editor->set_source("");
    assembled = false;
    update_ui_state();
}

void MainWindow::on_open_file() {
    QString file_name = QFileDialog::getOpenFileName(this, "Open Assembly File", "", "Assembly (*.asm *.s);;All Files (*.*)");
    if (!file_name.isEmpty()) {
        on_load_example(file_name);
        
        QSettings settings("memu8086", "memu8086");
        QStringList recent = settings.value("recentFiles").toStringList();
        recent.removeAll(file_name);
        recent.prepend(file_name);
        while (recent.size() > 5) recent.removeLast();
        settings.setValue("recentFiles", recent);
        update_recent_files_menu();
    }
}

void MainWindow::on_load_example(const QString& source_file) {
    QFile file(source_file);
    if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QTextStream in(&file);
        editor->set_source(in.readAll());
        current_file = source_file;
        assembled = false;
        editor->mark_saved();
        update_ui_state();
    }
}

void MainWindow::on_save_file() {
    if (current_file.isEmpty()) {
        on_save_as();
    } else {
        QFile file(current_file);
        if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
            QTextStream out(&file);
            out << editor->get_source();
            editor->mark_saved();
            update_ui_state();
        }
    }
}

void MainWindow::on_save_as() {
    QString file_name = QFileDialog::getSaveFileName(this, "Save Assembly File", "", "Assembly (*.asm *.s);;All Files (*.*)");
    if (!file_name.isEmpty()) {
        current_file = file_name;
        on_save_file();
        
        QSettings settings("memu8086", "memu8086");
        QStringList recent = settings.value("recentFiles").toStringList();
        recent.removeAll(file_name);
        recent.prepend(file_name);
        while (recent.size() > 5) recent.removeLast();
        settings.setValue("recentFiles", recent);
        update_recent_files_menu();
    }
}

void MainWindow::show_about() {
    QDialog dlg(this);
    dlg.setWindowTitle("About memu8086");
    dlg.setFixedSize(400, 300);

    QVBoxLayout* layout = new QVBoxLayout(&dlg);
    layout->setAlignment(Qt::AlignHCenter | Qt::AlignTop);

    QLabel* title = new QLabel("<b>memu8086</b> v1.0.0");
    title->setFont(Theme::ui_font(18));
    title->setAlignment(Qt::AlignCenter);

    QLabel* desc = new QLabel("A modern Intel 8086 emulator & assembler");
    desc->setStyleSheet(QStringLiteral("color: %1;").arg(Theme::Color::TEXT_MUTED));
    desc->setAlignment(Qt::AlignCenter);

    QLabel* built = new QLabel("Built with: Qt6, C++17");
    built->setAlignment(Qt::AlignCenter);

    QFrame* line = new QFrame();
    line->setFrameShape(QFrame::HLine);
    line->setFrameShadow(QFrame::Sunken);
    line->setStyleSheet(QStringLiteral("background-color: %1; max-height: 1px;").arg(Theme::Color::BORDER));

    QGridLayout* grid = new QGridLayout();
    QStringList keys = {"F5", "F9", "F7", "F8", "Ctrl+O", "Ctrl+S"};
    QStringList actions = {"Assemble", "Run/Stop", "Step", "Step Over", "Open File", "Save"};
    for (int i = 0; i < keys.size(); ++i) {
        QLabel* lbl_k = new QLabel("<b>" + keys[i] + "</b>");
        QLabel* lbl_a = new QLabel(actions[i]);
        lbl_k->setAlignment(Qt::AlignRight);
        grid->addWidget(lbl_k, i/2, (i%2)*2);
        grid->addWidget(lbl_a, i/2, (i%2)*2 + 1);
    }

    QPushButton* btn_close = new QPushButton("Close");
    btn_close->setFixedWidth(100);
    connect(btn_close, &QPushButton::clicked, &dlg, &QDialog::accept);

    layout->addSpacing(10);
    layout->addWidget(title);
    layout->addWidget(desc);
    layout->addWidget(built);
    layout->addSpacing(10);
    layout->addWidget(line);
    layout->addSpacing(10);
    layout->addLayout(grid);
    layout->addStretch();
    layout->addWidget(btn_close, 0, Qt::AlignCenter);

    dlg.exec();
}

void MainWindow::show_settings() {
    SettingsDialog dlg(this);
    connect(&dlg, &SettingsDialog::settings_applied, this, [this]() {
        // Re-apply editor font if size changed
        QSettings s("memu8086", "memu8086");
        int font_size = s.value("editor/font_size", 13).toInt();
        editor->set_font_size(font_size);
    });
    dlg.exec();
}

void MainWindow::show_examples() {
    ExamplesDialog dlg(this);
    connect(&dlg, &ExamplesDialog::load_example, this, [this](const QString& source) {
        on_new_file();
        editor->set_source(source);
    });
    dlg.exec();
}

void MainWindow::closeEvent(QCloseEvent* event) {
    QSettings s("memu8086", "memu8086");
    
    // Only update geometry and state if the user explicitly opted into saving layouts
    if (s.value("layout_user_saved", false).toBool()) {
        s.setValue("geometry", saveGeometry());
        s.setValue("windowState", saveState());
    }
    s.setValue("lastFile", current_file);
    event->accept();
}

} // namespace memu8086::ui