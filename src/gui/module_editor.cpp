#include "stdafx.h"

#include "misc/config.h"

#include "_module_entry.hpp"
#include "module_editor.h"

namespace taolog {

void ModuleEntryEditor::get_metas(WindowMeta * metas)
{
    __super::get_metas(metas);
    metas->style &= ~(WS_MINIMIZEBOX);
}

LPCTSTR ModuleEntryEditor::get_skin_xml() const
{
    LPCTSTR json = LR"tw(
<Window title="添加模块" size="350,200">
    <Resource>
        <Font name="default" face="微软雅黑" size="12"/>
        <Font name="1" face="微软雅黑" size="12"/>
        <Font name="consolas" face="Consolas" size="12"/>
    </Resource>
    <Root>
        <Vertical>
            <Vertical name="container" padding="10,10,10,10" height="144">
                <Horizontal height="30" padding="0,3,0,3">
                    <Label style="centerimage" text="名字" width="50"/>
                    <TextBox name="name" style="tabstop" exstyle="clientedge"/>
                </Horizontal>
                <Horizontal height="30" padding="0,3,0,3">
                    <Label style="centerimage" text="GUID" width="50"/>
                    <TextBox name="guid" style="tabstop" exstyle="clientedge" />
                </Horizontal>
                <Horizontal height="30" padding="0,3,0,3">
                    <Label style="centerimage" text="目录" width="50"/>
                    <TextBox name="root" style="tabstop" exstyle="clientedge" />
                </Horizontal>
                <Horizontal height="30" padding="0,3,0,3">
                    <Label style="centerimage" text="等级" width="50"/>
                    <ComboBox name="level" style="tabstop" height="200"/>
                </Horizontal>
            </Vertical>
            <Horizontal height="40" padding="10,4,10,4">
                <Control />
                <Button name="ok" text="保存" width="50" style="tabstop,default"/>
                <Control width="10" />
                <Button name="cancel" text="取消" width="50" style="tabstop"/>
            </Horizontal>
        </Vertical>
    </Root>
</Window>
)tw";
    return json;
}

LRESULT ModuleEntryEditor::handle_message(UINT umsg, WPARAM wparam, LPARAM lparam)
{
    switch (umsg) {
    case WM_CREATE:
    {
        ::SetWindowText(_hwnd, !_mod ? L"添加模块" : L"修改模块");

        _name = _root->find<taowin::TextBox>(L"name");
        _path = _root->find<taowin::TextBox>(L"root");
        _level = _root->find<taowin::ComboBox>(L"level");
        _guid = _root->find<taowin::TextBox>(L"guid");
        _ok = _root->find<taowin::Button>(L"ok");
        _cancel = _root->find<taowin::Button>(L"cancel");

        for (auto& pair : *_levels) {
            int i = _level->add_string(pair.second.cmt2.c_str());
            _level->set_item_data(i, (void*)pair.first);
        }

        if (!_mod) {
            _level->set_cur_sel(1);
        }
        else {
            _name->set_text(_mod->name.c_str());
            _path->set_text(_mod->root.c_str());

            wchar_t guid[128];
            if (::StringFromGUID2(_mod->guid, &guid[0], _countof(guid))) {
                _guid->set_text(guid);
            }
            else {
                _guid->set_text(L"{00000000-0000-0000-0000-000000000000}");
            }

            for (int i = 0, n = _level->get_count();; i++) {
                if (i < n) {
                    if ((int)_level->get_item_data(i) == _mod->level) {
                        _level->set_cur_sel(i);
                        break;
                    }
                }
                else {
                    _level->set_cur_sel(1);
                    break;
                }
            }
        }

        _name->focus();

        return 0;
    }
    }
    return __super::handle_message(umsg, wparam, lparam);
}

LRESULT ModuleEntryEditor::on_notify(HWND hwnd, taowin::Control * pc, int code, NMHDR * hdr)
{
    if (!pc) return 0;

    if (pc->name() == L"ok") {
        if(code == BN_CLICKED) {
            _on_ok();
        }
    }
    else if (pc->name() == L"cancel") {
        if(code == BN_CLICKED) {
            close();
            return 0;
        }
    }

    return 0;
}

bool ModuleEntryEditor::filter_special_key(int vk)
{
    if (vk == VK_RETURN) {
        _on_ok();
        return true;
    }

    return __super::filter_special_key(vk);
}

int ModuleEntryEditor::_on_ok()
{
    if (!_validate_form())
        return 0;

    auto entry = _mod ? _mod : new ModuleEntry;
    entry->name = _name->get_text();
    entry->root = _path->get_text();
    entry->level = (unsigned char)(int)_level->get_item_data(_level->get_cur_sel());
    entry->guid_str = _guid->get_text();
    ::CLSIDFromString(entry->guid_str.c_str(), &entry->guid);

    if (!_mod) entry->enable = false;

    _onok(entry);

    close();

    return 0;
}

bool ModuleEntryEditor::_validate_form()
{
    if (_name->get_text() == L"") {
        msgbox(L"模块名字不应为空。", MB_ICONERROR);
        _name->focus();
        return false;
    }

    auto guid = _guid->get_text();
    CLSID clsid;
    if (FAILED(::CLSIDFromString(guid.c_str(), &clsid)) || ::IsEqualGUID(clsid, GUID_NULL)) {
        msgbox(L"无效 GUID 值。", MB_ICONERROR);
        _guid->focus();
        return false;
    }

    std::wstring err;
    if (!_oncheckguid(clsid, &err)) {
        msgbox(err.c_str(), MB_ICONERROR);
        _guid->focus();
        return false;
    }

    return true;
}

}

