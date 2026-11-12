#include <QApplication>
#include <QGenericPlugin>
#include <QAction>
#include <QAbstractButton>
#include <QCheckBox>
#include <QComboBox>
#include <QDialog>
#include <QDockWidget>
#include <QEvent>
#include <QFontDatabase>
#include <QGroupBox>
#include <QHash>
#include <QLabel>
#include <QLineEdit>
#include <QTextBrowser>
#include <QListWidget>
#include <QMenu>
#include <QMenuBar>
#include <QSet>
#include <QSignalBlocker>
#include <QTimer>
#include <QToolBar>
#include <QTreeWidget>

namespace {
struct Term { const char *en; const char *zh; };
constexpr Term terms[] = {
#include "terms.inc"
};
const QHash<QString,QString>& dictionary() {
    static const auto table = [] {
        QHash<QString,QString> result;
        for (const auto& t : terms)
            result.insert(QString::fromUtf8(t.en), QString::fromUtf8(t.zh));
        return result;
    }();
    return table;
}
QString localized(QString value) {
    const auto& table = dictionary();
    if(value.isEmpty()) return value;
    if (auto it = table.constFind(value); it != table.cend()) return it.value();
    if (value.startsWith("Last Filter: ")) return QStringLiteral("上次滤镜：") + localized(value.mid(13));
    const auto original = value;
    QString mnemonic;
    const auto at = value.indexOf('&');
    if (at >= 0 && at + 1 < value.size() && value[at+1].isLetter() && value[at+1].unicode() < 128) {
        mnemonic = "(&" + QString(value[at+1].toUpper()) + ")";
        value.remove(at,1);
    }
    QString tail;
    if (value.endsWith(QChar(0x2026))) { value.chop(1); tail = QStringLiteral("…"); }
    const auto it = table.constFind(value);
    if (it == table.cend()) return original;
    return it.value() + mnemonic + tail;
}
void translateAction(QAction* a) {
    if (!a || a->isSeparator() || a->objectName()=="recentProject") return;
    // The editor uses this literal to resync its checked state across projects.
    if (a->text()=="Pixel Grid" && a->objectName()!="pixelGrid") return;
    const auto label=localized(a->text());
    if (label!=a->text()) a->setText(label);
    const auto tip=localized(a->toolTip());
    if (tip!=a->toolTip()) a->setToolTip(tip);
}
void translateWidget(QWidget* w) {
    if (auto* menu=qobject_cast<QMenu*>(w)) {
        translateAction(menu->menuAction());
    }
    if (qobject_cast<QMenuBar*>(w) || qobject_cast<QMenu*>(w) || qobject_cast<QToolBar*>(w))
        for(auto* a:w->actions()) translateAction(a);
    if(auto* b=qobject_cast<QAbstractButton*>(w)) {
        const auto label=(b->objectName()=="exportCmykProfile" && !b->text().startsWith("Choose CMYK profile")) ? b->text() : localized(b->text());
        if(label!=b->text() && !(qobject_cast<QCheckBox*>(b) && b->objectName().isEmpty()))
            b->setText(label);
    } else if(auto* l=qobject_cast<QLabel*>(w)) {
        const auto s=localized(l->text());
        if(s!=l->text()) l->setText(s);
    } else if(auto* group=qobject_cast<QGroupBox*>(w)) {
        const auto s=localized(group->title());
        if(s!=group->title()) group->setTitle(s);
    } else if(auto* line=qobject_cast<QLineEdit*>(w)) {
        const auto s=localized(line->placeholderText());
        if(s!=line->placeholderText()) line->setPlaceholderText(s);
    } else if(auto* c=qobject_cast<QComboBox*>(w)) {
        // Only controls whose values in v1.5.9 are IDs, item data or indices.
        static const QSet<QString> safe{"newCanvasUnit","newCanvasBackground","newCanvasPreset","layerBlend",
            "lensEffectMode","lensBokehShape","exportFormat","selectionMode","lassoKind","gradientShape","gradientStyle","shapeKind","shapeLineStyle","shapeLineCap"};
        if(safe.contains(c->objectName())) {
            QSignalBlocker block(c);
            for(int i=0;i<c->count();++i) {
                const auto s=localized(c->itemText(i));
                if(s!=c->itemText(i)) c->setItemText(i,s);
            }
        }
    } else if(auto* browser=qobject_cast<QTextBrowser*>(w); browser && browser->objectName()=="whatsNewContent" &&
              browser->toPlainText().contains("Add editable Vignette, Tonal Contrast and Lens Effects adjustment layers.")) {
        browser->setMarkdown(QStringLiteral("## 新增功能\n\n新增可编辑的暗角、色调对比度和镜头效果调整图层。设置、蒙版、不透明度和历史记录均随图层保留，原始像素保持不变。Windows 项目格式分别使用 v20 保存暗角、v21 保存色调对比度、v22 保存镜头效果。\n\n## 改进\n\n应用图标换用了新版 Compositor 图案。编辑镜头效果时，预览可保留更多图像细节。\n\n## 修复\n\n画笔和橡皮擦的笔画只要产生改动，就会单独记录一步撤销，即使此时正在编辑不透明度或形状样式也是如此。绘制中按 Esc 会保留另一项编辑。绘制蒙版也遵循相同的历史记录规则。"));
    } else if(auto* list=qobject_cast<QListWidget*>(w); list && list->objectName()=="commandPaletteList") {
        for(int i=0;i<list->count();++i){
            auto* row=list->item(i);
            auto fields=row->text().split("    ");
            if(fields.size()<2) continue;
            auto label=fields[0];
            const bool checked=label.startsWith(QStringLiteral("✓ "));
            if(checked) label.remove(0,2);
            fields[0]=(checked?QStringLiteral("✓ "):QString())+localized(label);
            fields[1]=localized(fields[1]);
            const auto text=fields.join("    ");
            if(text!=row->text()) row->setText(text);
        }
    } else if(auto* tree=qobject_cast<QTreeWidget*>(w); tree && tree->objectName()=="shortcutList") {
        auto translateCell=[](QTreeWidgetItem* item,int col) {
            auto s=localized(item->text(col));
            if(s!=item->text(col)) item->setText(col,s);
        };
        for(int i=0;i<3;++i) translateCell(tree->headerItem(),i);
        for(int i=0;i<tree->topLevelItemCount();++i)
            for(int j=0;j<2;++j) translateCell(tree->topLevelItem(i),j);
    }
    if(qobject_cast<QDialog*>(w)||qobject_cast<QDockWidget*>(w)) {
        const auto title=w->windowTitle();
        const auto s=title.startsWith(QStringLiteral("RAW Development – ")) ?
            QStringLiteral("RAW 显影 – ")+title.mid(QStringLiteral("RAW Development – ").size()) : localized(title);
        if(s!=title) w->setWindowTitle(s);
    }
    const auto tip=localized(w->toolTip());
    if(tip!=w->toolTip()) w->setToolTip(tip);
}
void applyInterfaceFont() {
    const auto installed = QFontDatabase::families();
    const QStringList preferred{QStringLiteral("SimHei"), QStringLiteral("黑体"),
                                QStringLiteral("Microsoft YaHei UI"), QStringLiteral("Microsoft YaHei"),
                                QStringLiteral("微软雅黑"), QStringLiteral("Noto Sans CJK SC")};
    for (const auto& family : preferred) {
        if (!installed.contains(family, Qt::CaseInsensitive)) continue;
        QFont font = qApp->font();
        font.setFamilies({family});
        auto sheet = qApp->styleSheet();
        const auto original = qApp->property("studioFontFamily").toString();
        const QString rule = QStringLiteral("font-family: \"%1\"");
        if (!original.isEmpty() && sheet.contains(rule.arg(original)))
            sheet.replace(rule.arg(original), rule.arg(family));
        else
            sheet += QStringLiteral("\nQWidget { %1; }").arg(rule.arg(family));
        qApp->setFont(font);
        qApp->setProperty("studioFontFamily",family);
        qApp->setStyleSheet(sheet);
        return;
    }
}
class Localizer final : public QObject {
    bool busy=false;
public:
    void start() {
        if(qApp->applicationName()!="Compositor" || qApp->applicationVersion()!="1.5.9") return;
        applyInterfaceFont();
        for(auto* w:QApplication::allWidgets()) translateWidget(w);
        qApp->installEventFilter(this);
    }
    bool eventFilter(QObject* receiver,QEvent* e) override {
        if(busy) return false;
        switch(e->type()) {
        case QEvent::Paint:
            // Only small text widgets need this fallback for in-place caption changes.
            if(!qobject_cast<QLabel*>(receiver) && !qobject_cast<QAbstractButton*>(receiver)) break;
            [[fallthrough]];
        case QEvent::Show: case QEvent::LayoutRequest: case QEvent::ActionChanged:
        case QEvent::ToolTip: case QEvent::WindowTitleChange:
            if(auto* w=qobject_cast<QWidget*>(receiver)) {
                busy=true; translateWidget(w); busy=false;
            }
            break;
        default: break;
        }
        return false;
    }
};
}
class CompositorChinese final : public QGenericPlugin {
    Q_OBJECT
    Q_PLUGIN_METADATA(IID QGenericPluginFactoryInterface_iid FILE "plugin.json")
public:
    QObject* create(const QString& name,const QString&) override {
        if(name.compare("compositorzh",Qt::CaseInsensitive)!=0) return nullptr;
        auto* filter=new Localizer;
        QTimer::singleShot(0,filter,[filter]{filter->start();});
        return filter;
    }
};
#include "plugin.moc"
