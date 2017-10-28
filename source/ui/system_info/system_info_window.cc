//
// PROJECT:         Aspia Remote Desktop
// FILE:            ui/system_info/system_info_window.cc
// LICENSE:         Mozilla Public License Version 2.0
// PROGRAMMERS:     Dmitry Chapyshev (dmitry@aspia.ru)
//

#include "base/strings/unicode.h"
#include "base/logging.h"
#include "ui/system_info/system_info_window.h"
#include "ui/system_info/save_report_dialog.h"
#include "ui/about_dialog.h"

namespace aspia {

static const int kDefaultWindowWidth = 980;
static const int kDefaultWindowHeight = 700;

SystemInfoWindow::SystemInfoWindow(Delegate* delegate)
    : delegate_(delegate)
{
    DCHECK(delegate);
    ui_thread_.Start(MessageLoop::TYPE_UI, this);
}

SystemInfoWindow::~SystemInfoWindow()
{
    ui_thread_.Stop();
}

void SystemInfoWindow::OnBeforeThreadRunning()
{
    runner_ = ui_thread_.message_loop_proxy();
    DCHECK(runner_);

    CString title;
    title.LoadStringW(IDS_SI_SYSTEM_INFORMATION);

    if (!Create(nullptr, rcDefault, title, WS_OVERLAPPEDWINDOW))
    {
        LOG(ERROR) << "System information window not created: " << GetLastSystemErrorString();
        runner_->PostQuit();
    }
    else
    {
        ShowWindow(SW_SHOW);
        UpdateWindow();
    }
}

void SystemInfoWindow::OnThreadRunning(MessageLoop* message_loop)
{
    // Run message loop with this message dispatcher.
    message_loop->Run(this);
}

void SystemInfoWindow::OnAfterThreadRunning()
{
    DestroyWindow();
}

bool SystemInfoWindow::Dispatch(const NativeEvent& event)
{
    TranslateMessage(&event);
    DispatchMessageW(&event);
    return true;
}

LRESULT SystemInfoWindow::OnCreate(UINT message, WPARAM wparam, LPARAM lparam, BOOL& handled)
{
    UNUSED_PARAMETER(message);
    UNUSED_PARAMETER(wparam);
    UNUSED_PARAMETER(lparam);
    UNUSED_PARAMETER(handled);

    small_icon_ = AtlLoadIconImage(IDI_MAIN,
                                   LR_CREATEDIBSECTION,
                                   GetSystemMetrics(SM_CXSMICON),
                                   GetSystemMetrics(SM_CYSMICON));
    SetIcon(small_icon_, FALSE);

    big_icon_ = AtlLoadIconImage(IDI_MAIN,
                                 LR_CREATEDIBSECTION,
                                 GetSystemMetrics(SM_CXICON),
                                 GetSystemMetrics(SM_CYICON));
    SetIcon(big_icon_, TRUE);

    const DWORD toolbar_style = WS_CHILD | WS_VISIBLE | TBSTYLE_FLAT |
        TBSTYLE_LIST | TBSTYLE_TOOLTIPS;

    toolbar_.Create(*this, rcDefault, nullptr, toolbar_style);

    CRect client_rect;
    GetClientRect(client_rect);

    splitter_.Create(*this, client_rect, nullptr,
                     WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN);

    splitter_.SetActivePane(SPLIT_PANE_LEFT);
    splitter_.SetSplitterPos(350);
    splitter_.m_cxySplitBar = 5;
    splitter_.m_cxyMin = 0;
    splitter_.m_bFullDrag = false;

    const DWORD tree_style = WS_CHILD | WS_VISIBLE | WS_TABSTOP | TVS_HASLINES |
        TVS_SHOWSELALWAYS | TVS_HASBUTTONS | TVS_LINESATROOT;

    tree_.Create(splitter_, rcDefault, nullptr, tree_style, WS_EX_CLIENTEDGE, kTreeControl);

    const DWORD list_style = WS_CHILD | WS_VISIBLE | LVS_REPORT | WS_TABSTOP | LVS_SHOWSELALWAYS;

    list_.Create(splitter_, rcDefault, nullptr, list_style, WS_EX_CLIENTEDGE, kListControl);

    splitter_.SetSplitterPane(SPLIT_PANE_LEFT, tree_);
    splitter_.SetSplitterPane(SPLIT_PANE_RIGHT, list_);

    statusbar_.Create(*this, rcDefault, nullptr, WS_CHILD | WS_VISIBLE | SBARS_SIZEGRIP);
    int part_width = 240;
    statusbar_.SetParts(1, &part_width);

    SetWindowPos(nullptr, 0, 0, kDefaultWindowWidth, kDefaultWindowHeight,
                 SWP_NOZORDER | SWP_NOACTIVATE | SWP_NOMOVE);
    CenterWindow();

    tree_.SelectItem(tree_.GetChildItem(TVI_ROOT));
    tree_.SetFocus();
    return 0;
}

LRESULT SystemInfoWindow::OnDestroy(UINT message, WPARAM wparam, LPARAM lparam, BOOL& handled)
{
    UNUSED_PARAMETER(message);
    UNUSED_PARAMETER(wparam);
    UNUSED_PARAMETER(lparam);
    UNUSED_PARAMETER(handled);

    toolbar_.DestroyWindow();
    tree_.DestroyWindow();
    list_.DestroyWindow();
    splitter_.DestroyWindow();
    statusbar_.DestroyWindow();
    return 0;
}

LRESULT SystemInfoWindow::OnSize(UINT message, WPARAM wparam, LPARAM lparam, BOOL& handled)
{
    UNUSED_PARAMETER(message);
    UNUSED_PARAMETER(wparam);
    UNUSED_PARAMETER(handled);

    const CSize size(lparam);

    toolbar_.AutoSize();
    statusbar_.SendMessageW(WM_SIZE);

    CRect toolbar_rect;
    toolbar_.GetWindowRect(toolbar_rect);

    CRect statusbar_rect;
    statusbar_.GetWindowRect(statusbar_rect);

    splitter_.MoveWindow(0, toolbar_rect.Height(),
                         size.cx, size.cy - toolbar_rect.Height() - statusbar_rect.Height(),
                         FALSE);
    return 0;
}

LRESULT SystemInfoWindow::OnGetMinMaxInfo(UINT message, WPARAM wparam, LPARAM lparam,
                                          BOOL& handled)
{
    UNUSED_PARAMETER(message);
    UNUSED_PARAMETER(wparam);
    UNUSED_PARAMETER(handled);

    LPMINMAXINFO mmi = reinterpret_cast<LPMINMAXINFO>(lparam);

    mmi->ptMinTrackSize.x = 500;
    mmi->ptMinTrackSize.y = 400;

    return 0;
}

LRESULT SystemInfoWindow::OnClose(UINT message, WPARAM wparam, LPARAM lparam, BOOL& handled)
{
    UNUSED_PARAMETER(message);
    UNUSED_PARAMETER(wparam);
    UNUSED_PARAMETER(lparam);
    UNUSED_PARAMETER(handled);

    delegate_->OnWindowClose();
    return 0;
}

void SystemInfoWindow::ShowDropDownMenu(int button_id, RECT* button_rect)
{
    if (button_id != ID_SAVE)
        return;

    if (toolbar_.MapWindowPoints(HWND_DESKTOP, button_rect))
    {
        TPMPARAMS tpm;
        tpm.cbSize = sizeof(TPMPARAMS);
        tpm.rcExclude = *button_rect;

        CMenu menu;
        menu.LoadMenuW(IDR_SAVE_REPORT);

        CMenuHandle pupup = menu.GetSubMenu(0);

        pupup.TrackPopupMenuEx(TPM_LEFTALIGN | TPM_LEFTBUTTON | TPM_VERTICAL,
                               button_rect->left,
                               button_rect->bottom,
                               *this,
                               &tpm);
    }
}

LRESULT SystemInfoWindow::OnToolBarDropDown(int control_id, LPNMHDR hdr, BOOL& handled)
{
    UNUSED_PARAMETER(control_id);
    UNUSED_PARAMETER(handled);

    LPNMTOOLBARW header = reinterpret_cast<LPNMTOOLBARW>(hdr);
    ShowDropDownMenu(header->iItem, &header->rcButton);
    return 0;
}

LRESULT SystemInfoWindow::OnCategorySelected(int control_id, LPNMHDR hdr, BOOL& handled)
{
    UNUSED_PARAMETER(control_id);
    UNUSED_PARAMETER(handled);

    LPNMTREEVIEWW nmtv = reinterpret_cast<LPNMTREEVIEWW>(hdr);

    list_.DeleteAllItems();
    list_.DeleteAllColumns();

    const CategoryTreeCtrl::ItemType type = tree_.GetItemType(nmtv->itemNew.hItem);

    if (type == CategoryTreeCtrl::ItemType::CATEGORY)
    {
        CategoryInfo* category = tree_.GetItem(nmtv->itemNew.hItem)->category_info();
        if (!category)
            return 0;

        statusbar_.SetText(0, UNICODEfromUTF8(category->Name()).c_str());
        statusbar_.SetIcon(0, AtlLoadIconImage(category->Icon(),
                                               LR_CREATEDIBSECTION,
                                               GetSystemMetrics(SM_CXSMICON),
                                               GetSystemMetrics(SM_CYSMICON)));

        // Prepare the list of requested categories.
        Delegate::GuidList guid_list;
        guid_list.emplace_back(category->Guid());

        delegate_->OnRequest(std::move(guid_list), list_.output_proxy());
    }
    else if (type == CategoryTreeCtrl::ItemType::GROUP)
    {
        CategoryGroup* group = tree_.GetItem(nmtv->itemNew.hItem)->category_group();
        if (!group)
            return 0;
    }
    else
    {
        DLOG(FATAL) << "Unexpected item type: " << static_cast<int>(type);
    }

    return 0;
}

LRESULT SystemInfoWindow::OnSaveSelectedButton(WORD notify_code, WORD control_id, HWND control,
                                               BOOL& handled)
{
    UNUSED_PARAMETER(notify_code);
    UNUSED_PARAMETER(control_id);
    UNUSED_PARAMETER(control);
    UNUSED_PARAMETER(handled);

    SaveReportDialog().DoModal(*this);
    return 0;
}

LRESULT SystemInfoWindow::OnAboutButton(WORD notify_code, WORD control_id, HWND control,
                                        BOOL& handled)
{
    UNUSED_PARAMETER(notify_code);
    UNUSED_PARAMETER(control_id);
    UNUSED_PARAMETER(control);
    UNUSED_PARAMETER(handled);

    AboutDialog().DoModal(*this);
    return 0;
}

LRESULT SystemInfoWindow::OnExitButton(WORD notify_code, WORD control_id, HWND control,
                                       BOOL& handled)
{
    UNUSED_PARAMETER(notify_code);
    UNUSED_PARAMETER(control_id);
    UNUSED_PARAMETER(control);
    UNUSED_PARAMETER(handled);

    PostMessageW(WM_CLOSE);
    return 0;
}

} // namespace aspia