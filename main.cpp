#include "./include/adb.hpp"
#include "./include/file.hpp"
#include "include/data.h"
#include "include/setting.hpp"
#include "wx/dynarray.h"
#include "wx/event.h"
#include "wx/language.h"
#include "wx/msgdlg.h"
#include "wx/string.h"
#include <algorithm>
#include <string>
#include <wx/filename.h>
#include <wx/wxprec.h>

#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif

std::vector<FileManager::FileList> init();

class MyApp : public wxApp {
public:
  virtual bool OnInit();
};

enum { ID_Export = 1001, ID_Import, ID_Setting };

class MyFrame : public wxFrame {
public:
  MyFrame();

private:
  std::vector<FileManager::FileList> lists;
  wxListBox *deviceList;
  wxListBox *fileList;
  void OnDeviceSelected(wxCommandEvent &event);
  void OnExit(wxCommandEvent &event);
  void OnAbout(wxCommandEvent &event);
  void OnSettingExportPath(wxCommandEvent &event);
  void OnImportFromComputer(wxCommandEvent &event);
  void OnRefresh(wxCommandEvent &event);
  void OnFileSelected(wxCommandEvent &event);
  void OnFileListRightClick(wxMouseEvent &event);
  void OnExport(wxCommandEvent &event);
  void OnDelete(wxCommandEvent &event);
  void OnSendToDynamicDevice(wxCommandEvent &event);
};

wxIMPLEMENT_APP(MyApp);

bool MyApp::OnInit() {
  wxLocale locale;
  locale.Init(wxLANGUAGE_CHINESE_SIMPLIFIED);
  MyFrame *frame = new MyFrame();
  frame->Show(true);
  return true;
}

MyFrame::MyFrame()
    : wxFrame(NULL, wxID_ANY,
              wxString::FromUTF8("火影忍者手游录像管理器 TPPPP开发"),
              wxDefaultPosition, wxSize(1024, 768)) {

  // UI初始化
  SetIcon(wxICON(NarutoRecordManager));
  // 工具栏
  wxMenu *controlMenu = new wxMenu;
  controlMenu->Append(wxID_REFRESH, wxString::FromUTF8("刷新\tF5"));
  controlMenu->Append(ID_Import, wxString::FromUTF8("从电脑导入"));

  wxMenu *helpMenu = new wxMenu;
  helpMenu->Append(wxID_ABOUT, wxString::FromUTF8("关于"));

  wxMenu *settingMenu = new wxMenu;
  settingMenu->Append(ID_Setting, wxString::FromUTF8("导出路径"));

  wxMenuBar *menuBar = new wxMenuBar;
  menuBar->Append(controlMenu, wxString::FromUTF8("操作"));
  menuBar->Append(helpMenu, wxString::FromUTF8("帮助"));
  menuBar->Append(settingMenu, wxString::FromUTF8("设置"));

  SetMenuBar(menuBar);

  CreateStatusBar();

  // 设备列表
  wxBoxSizer *topSizer = new wxBoxSizer(wxHORIZONTAL);

  wxStaticText *deviceLabel =
      new wxStaticText(this, wxID_ANY, wxString::FromUTF8("设备列表"),
                       wxDefaultPosition, wxDefaultSize, wxALIGN_CENTER);
  deviceList = new wxListBox(this, wxID_ANY, wxDefaultPosition, wxDefaultSize);

  wxBoxSizer *leftSizer = new wxBoxSizer(wxVERTICAL);
  leftSizer->Add(deviceLabel, 0, wxALIGN_CENTER_HORIZONTAL | wxTOP | wxBOTTOM,
                 10);
  leftSizer->Add(deviceList, 1, wxEXPAND | wxLEFT | wxRIGHT | wxBOTTOM, 10);

  // 文件列表
  wxStaticText *fileLabel =
      new wxStaticText(this, wxID_ANY, wxString::FromUTF8("文件列表"),
                       wxDefaultPosition, wxDefaultSize, wxALIGN_CENTER);
  fileList = new wxListBox(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, 0,
                           nullptr, wxLB_MULTIPLE);

  wxBoxSizer *rightSizer = new wxBoxSizer(wxVERTICAL);
  rightSizer->Add(fileLabel, 0, wxALIGN_CENTER_HORIZONTAL | wxTOP | wxBOTTOM,
                  10);
  rightSizer->Add(fileList, 1, wxEXPAND | wxLEFT | wxRIGHT | wxBOTTOM, 10);

  topSizer->Add(leftSizer, 1, wxEXPAND | wxALL, 5);
  topSizer->Add(rightSizer, 1, wxEXPAND | wxALL, 5);

  SetSizer(topSizer);
  Layout();

  deviceList->Bind(wxEVT_LISTBOX, &MyFrame::OnDeviceSelected, this);
  fileList->Bind(wxEVT_LISTBOX, &MyFrame::OnFileSelected, this);
  fileList->Bind(wxEVT_RIGHT_DOWN, &MyFrame::OnFileListRightClick, this);
  Bind(wxEVT_MENU, &MyFrame::OnSettingExportPath, this, ID_Setting);
  Bind(wxEVT_MENU, &MyFrame::OnAbout, this, wxID_ABOUT);
  Bind(wxEVT_MENU, &MyFrame::OnImportFromComputer, this, ID_Import);
  Bind(wxEVT_MENU, &MyFrame::OnExit, this, wxID_EXIT);
  Bind(wxEVT_MENU, &MyFrame::OnRefresh, this, wxID_REFRESH);

  // 程序初始化
  SetStatusText(wxString::FromUTF8("初始化中......"));
  lists = init();

  for (const auto &i : lists) {
    deviceList->Append(i.device_id);
  }

  SetStatusText(wxString::FromUTF8("初始化完成！"));
}

void MyFrame::OnExit(wxCommandEvent &event) { Close(true); }

void MyFrame::OnAbout(wxCommandEvent &event) {
  wxMessageBox(
      wxString::FromUTF8("该程序基于Android Debug "
                         "Bridge，实现火影忍者手游录像的管理迁移等功能\n开发者"
                         "：TPPPP\nBilibili:https://space.bilibili.com/"
                         "358783831\nGithub:https://github.com/TPPPP72/"
                         "NarutoRecordManager\n\n版本号:0.2"),
      wxString::FromUTF8("关于该软件"), wxOK | wxICON_INFORMATION);
}

void MyFrame::OnSettingExportPath(wxCommandEvent &event) {
  wxDirDialog dirDialog(this, wxString::FromUTF8("选择导出的文件夹"), "",
                        wxDD_DEFAULT_STYLE | wxDD_DIR_MUST_EXIST);
  if (dirDialog.ShowModal() == wxID_OK) {
    Setting::WriteData(Setting::Data{dirDialog.GetPath().ToStdString()});
    SetStatusText(wxString::FromUTF8("设置导出文件夹为:") +
                  dirDialog.GetPath());
  } else {
    wxMessageBox(wxString::FromUTF8("未选择文件夹"), wxString::FromUTF8("错误"),
                 wxOK | wxICON_ERROR);
  }
}

void MyFrame::OnRefresh(wxCommandEvent &event) {
  deviceList->SetSelection(wxNOT_FOUND);
  deviceList->Clear();
  fileList->SetSelection(wxNOT_FOUND);
  fileList->Clear();
  lists = init();

  for (const auto &item : lists) {
    deviceList->Append(item.device_id);
  }

  SetStatusText(wxString::FromUTF8("刷新完成！"));
}

void MyFrame::OnImportFromComputer(wxCommandEvent &event) {
  if (deviceList->GetSelection() == wxNOT_FOUND) {
    wxMessageBox(wxString::FromUTF8("请先在设备列表选择一个设备"),
                 wxString::FromUTF8("错误"), wxOK | wxICON_ERROR);
    return;
  }
  std::string selected_device_id =
      deviceList->GetString(deviceList->GetSelection()).ToStdString();
  wxFileDialog openFileDialog(
      this, wxString::FromUTF8("选择文件"),
      wxFileName::DirName(Setting::GetData().Export_Path).GetAbsolutePath(), "",
      wxString::FromUTF8("所有文件 (*.*)|*.*"),
      wxFD_OPEN | wxFD_FILE_MUST_EXIST | wxFD_MULTIPLE);

  if (openFileDialog.ShowModal() == wxID_OK) {
    wxArrayString paths;
    openFileDialog.GetPaths(paths);
    int cnt = 0;
    for (const auto &path : paths) {
      SetStatusText(wxString::FromUTF8("正在导入文件:" + path));
      if (ADB::PushRemoteFile_Full(
              FileManager::GetListByDeviceID(lists, selected_device_id),
              path.ToStdString())) {
        ++cnt;
        fileList->Append(path.substr(path.rfind("\\") + 1));
      }
    }
    SetStatusText(
        wxString::FromUTF8("导入完成！ " + std::to_string(cnt) + "成功 " +
                           std::to_string(paths.size() - cnt) + "失败"));
  } else {
    wxMessageBox(wxString::FromUTF8("未选择文件"), wxString::FromUTF8("错误"),
                 wxOK | wxICON_ERROR);
    return;
  }
}

void MyFrame::OnDeviceSelected(wxCommandEvent &event) {
  fileList->SetSelection(wxNOT_FOUND);
  fileList->Clear();
  std::string selected_device_id =
      deviceList->GetString(event.GetSelection()).ToStdString();
  SetStatusText(wxString::FromUTF8("选择设备:" + selected_device_id));
  auto list = FileManager::GetListByDeviceID(lists, selected_device_id);
  for (const auto &item : list.record) {
    fileList->Append(item);
  }
  for (const auto &item : list.recordlist) {
    fileList->Append(item);
  }
}

void MyFrame::OnFileSelected(wxCommandEvent &event) {
  int index = event.GetSelection();
  wxString selected_file = fileList->GetString(index);
  wxArrayInt selections;
  fileList->GetSelections(selections);
  if (selections.Index(index) != wxNOT_FOUND) {
    SetStatusText(wxString::FromUTF8("选择文件:" + selected_file));
  } else {
    SetStatusText(wxString::FromUTF8("取消选择:" + selected_file));
  }
}

void MyFrame::OnFileListRightClick(wxMouseEvent &event) {
  wxPoint pos_in_list = event.GetPosition();
  wxPoint pos_on_screen = fileList->ClientToScreen(pos_in_list);
  wxPoint pos_in_frame = this->ScreenToClient(pos_on_screen);

  int file_list_index = fileList->HitTest(pos_in_list);
  if (file_list_index != wxNOT_FOUND) {
    fileList->SetSelection(file_list_index);
    SetStatusText(wxString::FromUTF8("选择文件:" + fileList->GetString(file_list_index)));
  }
  wxMenu *menu=new wxMenu;
  menu->Append(ID_Export, wxString::FromUTF8("导出到电脑"));

  if (lists.size() >= 2) {
    wxMenu *sendSubMenu = new wxMenu;
    auto device_list_index = deviceList->GetSelection();
    for (int item = 0; item < lists.size(); item++) {
      if (lists[item].device_id !=
          deviceList->GetString(device_list_index).ToStdString()) {
        sendSubMenu->Append(2000 + item, lists[item].device_id);
        Bind(wxEVT_MENU, &MyFrame::OnSendToDynamicDevice, this, 2000 + item);
      }
    }
    menu->AppendSubMenu(sendSubMenu, wxString::FromUTF8("发送到其他设备"));
  }

  menu->Append(wxID_DELETE, wxString::FromUTF8("删除"));
  menu->Bind(wxEVT_MENU, &MyFrame::OnExport, this, ID_Export);
  menu->Bind(wxEVT_MENU, &MyFrame::OnDelete, this, wxID_DELETE);

  PopupMenu(menu, pos_in_frame);

  delete menu;
}

void MyFrame::OnExport(wxCommandEvent &event) {
  wxArrayInt selections;
  fileList->GetSelections(selections);
  int cnt = 0;
  for (auto &i : selections) {
    std::string file = fileList->GetString(i).ToStdString();
    std::string selected_device_id =
        deviceList->GetString(deviceList->GetSelection()).ToStdString();
    SetStatusText(wxString::FromUTF8("正在导出文件:" + file));
    if (ADB::PullRemoteFile(
            FileManager::GetListByDeviceID(lists, selected_device_id), file,
            Setting::GetData().Export_Path))
      ++cnt;
    // 进度显示待做
  }
  SetStatusText(
      wxString::FromUTF8("导出完成！ " + std::to_string(cnt) + "成功 " +
                         std::to_string(selections.size() - cnt) + "失败"));
}

void MyFrame::OnDelete(wxCommandEvent &event) {
  wxArrayInt selections;
  fileList->GetSelections(selections);
  int cnt = 0;
  std::vector<std::string> DelSuc;
  for (auto &item : selections) {
    std::string file = fileList->GetString(item).ToStdString();
    std::string selected_device_id =
        deviceList->GetString(deviceList->GetSelection()).ToStdString();
    SetStatusText(wxString::FromUTF8("正在删除文件:" + file));
    if (ADB::DeleteRemoteFile(
            FileManager::GetListByDeviceID(lists, selected_device_id), file)) {
      DelSuc.emplace_back(file);
      ++cnt;
    }
    // 进度显示待做
  }
  for (const auto &item : DelSuc) {
    fileList->Delete(fileList->FindString(item));
  }
  SetStatusText(
      wxString::FromUTF8("删除完成！ " + std::to_string(cnt) + "成功 " +
                         std::to_string(selections.size() - cnt) + "失败"));
}

void MyFrame::OnSendToDynamicDevice(wxCommandEvent &event) {
  wxArrayInt selections;
  fileList->GetSelections(selections);
  int cnt = 0;
  for (auto &i : selections) {
    std::string file = fileList->GetString(i).ToStdString();
    std::string selected_device_id =
        deviceList->GetString(deviceList->GetSelection()).ToStdString();
    SetStatusText(wxString::FromUTF8("正在发送文件:" + file));
    if (ADB::PullRemoteFile(
            FileManager::GetListByDeviceID(lists, selected_device_id), file) &&
        ADB::PushRemoteFile(lists[event.GetId() - 2000], file))
      ++cnt;
    // 进度显示待做
  }
  SetStatusText(
      wxString::FromUTF8("发送完成！ " + std::to_string(cnt) + "成功 " +
                         std::to_string(selections.size() - cnt) + "失败"));
  FileManager::local_system_clear();
}

std::vector<FileManager::FileList> init() {
  FileManager::local_system_init();
  auto devicelist = ADB::GetDevices();
  std::vector<FileManager::FileList> lists;
  std::for_each(devicelist.begin(), devicelist.end(), [&](const auto &item) {
    lists.emplace_back(FileManager::FileList{ADB::GetRecordFiles(item), item});
  });
  return lists;
}