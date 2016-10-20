#pragma once

#include <taowin/src/tw_taowin.h>

#include "_logdata_define.hpp"

#include "_module_entry.hpp"
#include "event_container.h"

#include "column_selection.h"
#include "module_manager.h"
#include "event_detail.h"
#include "result_filter.h"
#include "listview_color.h"

#include "controller.h"
#include "consumer.h"

#include "config.h"

namespace taoetw {

class MainWindow : public taowin::window_creator
{
public:

private:
    static const UINT kDoLog = WM_USER + 1;

public:
    MainWindow()
        : _listview(nullptr)
        , _btn_start(nullptr)
        , _btn_stop(nullptr)
        , _btn_clear(nullptr)
        , _btn_modules(nullptr)
        , _last_search_line(-1)
    {

    }
    
protected:
    JsonWrapper         _config;

    taowin::listview*   _listview;
    taowin::button*     _btn_start;
    taowin::button*     _btn_stop;
    taowin::button*     _btn_clear;
    taowin::button*     _btn_modules;
    taowin::button*     _btn_filter;
    taowin::button*     _btn_topmost;
    taowin::edit*       _edt_search;
    taowin::combobox*   _cbo_filter;
    taowin::button*     _btn_colors;

    MapColors           _colors;
    ColumnContainer     _columns;

    ModuleContainer     _modules;
    EventContainer      _events;
    EventContainerS     _filters;
    EventContainer*     _current_filter;
    ModuleLevelMap      _level_maps;

    int                 _last_search_line;
    std::wstring        _last_search_string;
    bool                _last_search_matched_cols[LogDataUI::data_cols];

    Controller          _controller;
    Consumer            _consumer;

    Guid2Module         _guid2mod;

protected:
    virtual LPCTSTR get_skin_xml() const override;
    virtual LRESULT handle_message(UINT umsg, WPARAM wparam, LPARAM lparam) override;
    virtual LRESULT control_message(taowin::syscontrol* ctl, UINT umsg, WPARAM wparam, LPARAM lparam) override;
    virtual LRESULT on_menu(int id, bool is_accel = false);
    virtual LRESULT on_notify(HWND hwnd, taowin::control* pc, int code, NMHDR* hdr) override;
    virtual bool filter_special_key(int vk) override;
    virtual void on_final_message() override { __super::on_final_message(); delete this; }

protected:
    bool _start();
    bool _stop();

    void _init_listview();
    void _init_config();
    void _view_detail(int i);
    void _manage_modules();
    void _show_filters();
    bool _do_search(const std::wstring& s, int line, int col);
    void _clear_results();
    void _set_top_most(bool top);
    void _update_main_filter();

    LRESULT _on_create();
    LRESULT _on_close();
    LRESULT _on_log(LogDataUI* log);
    LRESULT _on_custom_draw_listview(NMHDR* hdr);
    LRESULT _on_get_dispinfo(NMHDR* hdr);
    LRESULT _on_select_column();
    LRESULT _on_drag_column(NMHDR* hdr);
	void	_on_drop_column();

protected:
    void _module_from_guid(const GUID& guid, std::wstring* name, const std::wstring** root);
};

}
