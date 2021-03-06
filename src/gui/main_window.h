#pragma once

#include "misc/mem_pool.hpp"

#include "_module_entry.hpp"
#include "logdata_define.h"
#include "tooltip_window.h"

#include "listview_color.h"
#include "column_selection.h"
#include "event_container.h"
#include "event_detail.h"
#include "module_manager.h"
#include "result_filter.h"
#include "json_visual.h"
#include "lua_console.h"

#include "log/controller.h"
#include "log/consumer.h"
#include "log/dbgview.h"

namespace taolog {

struct LoggerMessage {
    enum Value {
        __Start = 0,
        LogEtwMsg,
        LogDbgMsg,
        LogRawDbgMsg,
        AllocLogUI,
    };
};

struct LogSysType
{
    enum Value
    {
        __Start,
        EventTracing,
        DebugView,
        __End,
    };
};

class MainWindow : public taowin::WindowCreator
{
private:
    static const UINT kDoLog = WM_USER + 3;
    static const UINT kLogDbgViewRaw = WM_USER + 4;

    class EventDataSource : public taowin::ListView::IDataSource
    {
    public:
        EventDataSource()
            : _events(nullptr)
        { }

        void SetEvents(EventContainer* events)
        {
            _events = events;
        }

        void SetColConv(std::function<int(int)> conv)
        {
            _col_conv = conv;
        }

    public:
        virtual size_t size() const override
        {
            return _events->size();
        }

        virtual LPCTSTR get(int item, int subitem) const override
        {
            auto idx = _col_conv(subitem);
            auto& evt = *(*_events)[item];
            auto str = evt[idx];
            return str;
        }

    protected:
        EventContainer* _events;
        std::function<int(int)> _col_conv;
    };

    class Filters2ComboBoxDataSource : public taowin::ComboBox::IDataSource
    {
    public:
        virtual size_t      Size() override
        {
            auto n = _data != nullptr ? _data->size() : 0;
            return n + 1;
        }

        virtual void        GetAt(size_t index, TCHAR const** text, void** tag) override
        {
            if(index == 0) {
                *text = L"全部";
                *tag = _all;
            }
            else {
                auto F = (*_data)[index-1];
                *text = F->name.c_str();
                *tag = F;
            }
        }

    public:
        Filters2ComboBoxDataSource()
            : _all(nullptr)
            , _data(nullptr)
        { }

        void set_source(EventContainer* all, EventContainerS* data)
        {
            _all = all;
            _data = data;
        }

    protected:
        EventContainer*  _all;
        EventContainerS* _data;
    };

public:
    MainWindow(LogSysType::Value type)
        : _logsystype(type)
    {
        _tipwnd = new TooltipWindow;
    }

    ~MainWindow()
    {
        // 日志结构体是由内存池管理的
        // 所以要强制手动释放，不能等到智能指针析构的时候进行
        _clear_results();

        _tipwnd = nullptr;
    }

    bool isetw() const { return _logsystype == LogSysType::EventTracing; }
    bool isdbg() const { return _logsystype == LogSysType::DebugView; }
    
protected:
    LogSysType::Value   _logsystype;
    JsonWrapper         _config;

    taowin::ListView*   _listview;
    taowin::TextBox*    _edt_search;
    taowin::ComboBox*   _cbo_search_filter;
    taowin::ComboBox*   _cbo_sel_flt;
    taowin::ComboBox*   _cbo_prj;
    taowin::Button*     _btn_tools;

    TooltipWindow*      _tipwnd;

    HACCEL              _accels;
    taowin::PopupMenu*  _lvmenu;
    taowin::PopupMenu*  _tools_menu;
    taowin::PopupMenu*  _main_menu;

    MapColors           _colors;
    ColumnManager       _columns;

    ModuleContainer     _modules;
    ModuleContainer2ComboBoxDataSource
                        _module_source;
    ModuleEntry*        _current_project;

    std::map<ModuleEntry*, EventPair> _projects;

    EventContainer*     _events;
    EventContainerS*    _filters;
    Filters2ComboBoxDataSource
                        _filter_source;
    EventContainer*     _current_filter;
    EventDataSource     _event_source;

    EventSearcher       _searcher;

    ModuleLevelMap      _level_maps;

    Controller          _controller;
    Consumer            _consumer;
    DebugView           _dbgview;

    Guid2Module         _guid2mod;

    MemPoolT<LogDataUI, 8192>
                        _log_pool;

    // 表示当前正在导出的模块（当前过滤器）
    // 正在导出时不能清空数据，也不能关闭窗口
    std::map<ModuleEntry*, bool>
                        _results_exporting;

    lua_State*          _lua;

protected:
    virtual void get_metas(WindowMeta* metas) override;
    virtual LPCTSTR get_skin_xml() const override;
    virtual LRESULT handle_message(UINT umsg, WPARAM wparam, LPARAM lparam) override;
    virtual LRESULT control_message(taowin::SystemControl* ctl, UINT umsg, WPARAM wparam, LPARAM lparam) override;
    virtual LRESULT on_menu(const taowin::MenuIDs& ids) override;
    virtual LRESULT on_accel(int id) override;
    virtual LRESULT on_notify(HWND hwnd, taowin::Control* pc, int code, NMHDR* hdr) override;
    virtual bool filter_special_key(int vk) override;
    virtual void on_final_message() override { __super::on_final_message(); delete this; }
    virtual bool filter_message(MSG* msg) override;
    virtual taowin::SystemControl* filter_control(HWND hwnd) override;

protected:
    bool _start();
    bool _stop();

    void _init_control_variables();
    void _init_control_events();
    void _init_listview();
    void _init_config();

    // 从配置文件初始化模块，并映射空的过滤器
    void _init_projects();

    // 从配置文件初始化模块过滤器
    void _init_filters();

    void _init_menus();
    void _init_project_events();
    void _init_filter_events();
    void _init_logger_events();
    void _view_detail(int i);
    void _manage_modules();
    void _show_filters();
    bool _do_search(bool first);
    void _clear_results();
    void _set_top_most(bool top);

    // 更新主界面搜索栏可用的列
    void _update_search_filter();

    // 导出当前过滤器内容到文件
    void _export2file();

    // 复制当前选中的行内容到剪贴板
    // 只复制第 1 个选中的行
    void _copy_selected_item();

protected:
    void _save_filters();

protected:
    LRESULT _on_create();
    LRESULT _on_close();
    LRESULT _on_log(LoggerMessage::Value msg, LPARAM lParam);
    LRESULT _log_raw_dbg(int pid, std::string* s);
    LRESULT _on_custom_draw_listview(NMHDR* hdr);
    LRESULT _on_select_column();
    LRESULT _on_drag_column(NMHDR* hdr);
	void	_on_drop_column();
    LRESULT _on_contextmenu(HWND hSender, int x, int y);
    LRESULT _on_init_popupmenu(HMENU hPopup);

protected:
    ModuleEntry* _module_from_guid(const GUID& guid);
};

}
