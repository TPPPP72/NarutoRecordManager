#include "./include/adb.hpp"
#include "./include/data.h"
#include "./include/file.hpp"
#include "./include/hexreader.hpp"
#include "./include/hexwriter.hpp"
#include "./include/setting.hpp"
#include "include/tools.hpp"
#include "include/winruntime.hpp"
#include "wx/app.h"
#include "wx/dynarray.h"
#include "wx/event.h"
#include "wx/language.h"
#include "wx/msgdlg.h"
#include "wx/string.h"
#include <algorithm>
#include <cstdio>
#include <format>
#include <fstream>
#include <memory>
#include <string>
#include <thread>
#include <wx/filename.h>
#include <wx/listctrl.h>
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
  wxListCtrl *fileList;
  void OnDeviceSelected(wxCommandEvent &event);
  void OnExit(wxCommandEvent &event);
  void OnAbout(wxCommandEvent &event);
  void OnADBWirelessDebugHelper(wxCommandEvent &event);
  void OnSettingExportPath(wxCommandEvent &event);
  void OnImportFromComputer(wxCommandEvent &event);
  void OnRefresh(wxCommandEvent &event);
  void OnFileSelected(wxListEvent &event);
  void OnFileListRightClick(wxMouseEvent &event);
  void OnExport(wxCommandEvent &event);
  void OnDelete(wxCommandEvent &event);
  void OnSendToDynamicDevice(wxCommandEvent &event);
  void OnClose(wxCloseEvent &event);
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
  controlMenu->Append(wxID_ADD, wxString::FromUTF8("ADB无线调试辅助"));
  controlMenu->Append(wxID_REFRESH, wxString::FromUTF8("刷新\tF5"));
  controlMenu->Append(ID_Import, wxString::FromUTF8("从电脑导入"));

  wxMenu *helpMenu = new wxMenu;
  helpMenu->Append(wxID_ABOUT, wxString::FromUTF8("关于"));

  wxMenu *settingMenu = new wxMenu;
  settingMenu->Append(ID_Setting, wxString::FromUTF8("导出路径"));

  wxMenuBar *menuBar = new wxMenuBar;
  menuBar->Append(controlMenu, wxString::FromUTF8("操作"));
  menuBar->Append(settingMenu, wxString::FromUTF8("设置"));
  menuBar->Append(helpMenu, wxString::FromUTF8("帮助"));

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
  fileList = new wxListCtrl(this, wxID_ANY, wxDefaultPosition, wxDefaultSize,
                            wxLC_REPORT);

  fileList->InsertColumn(0, wxString::FromUTF8("文件名"), wxLIST_FORMAT_LEFT,
                         140);
  fileList->InsertColumn(1, "1P", wxLIST_FORMAT_LEFT, 90);
  fileList->InsertColumn(2, "2P", wxLIST_FORMAT_LEFT, 90);
  fileList->InsertColumn(3, wxString::FromUTF8("胜负"), wxLIST_FORMAT_LEFT, 50);
  fileList->InsertColumn(4, wxString::FromUTF8("所属权"), wxLIST_FORMAT_LEFT,
                         150);

  wxBoxSizer *rightSizer = new wxBoxSizer(wxVERTICAL);
  rightSizer->Add(fileLabel, 0, wxALIGN_CENTER_HORIZONTAL | wxTOP | wxBOTTOM,
                  10);
  rightSizer->Add(fileList, 1, wxEXPAND | wxLEFT | wxRIGHT | wxBOTTOM, 10);

  topSizer->Add(leftSizer, 1, wxEXPAND | wxALL, 5);
  topSizer->Add(rightSizer, 1, wxEXPAND | wxALL, 5);

  SetSizer(topSizer);
  Layout();

  deviceList->Bind(wxEVT_LISTBOX, &MyFrame::OnDeviceSelected, this);
  fileList->Bind(wxEVT_LIST_ITEM_SELECTED, &MyFrame::OnFileSelected, this);
  fileList->Bind(wxEVT_LIST_ITEM_DESELECTED, &MyFrame::OnFileSelected, this);
  fileList->Bind(wxEVT_RIGHT_DOWN, &MyFrame::OnFileListRightClick, this);
  Bind(wxEVT_MENU, &MyFrame::OnADBWirelessDebugHelper, this, wxID_ADD);
  Bind(wxEVT_MENU, &MyFrame::OnSettingExportPath, this, ID_Setting);
  Bind(wxEVT_MENU, &MyFrame::OnAbout, this, wxID_ABOUT);
  Bind(wxEVT_MENU, &MyFrame::OnImportFromComputer, this, ID_Import);
  Bind(wxEVT_MENU, &MyFrame::OnExit, this, wxID_EXIT);
  Bind(wxEVT_MENU, &MyFrame::OnRefresh, this, wxID_REFRESH);
  Bind(wxEVT_CLOSE_WINDOW, &MyFrame::OnClose, this);

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

void MyFrame::OnADBWirelessDebugHelper(wxCommandEvent &event) {}

void MyFrame::OnSettingExportPath(wxCommandEvent &event) {
  wxDirDialog dirDialog(this, wxString::FromUTF8("选择导出的文件夹"), "",
                        wxDD_DEFAULT_STYLE | wxDD_DIR_MUST_EXIST);
  if (dirDialog.ShowModal() == wxID_OK) {
    Setting::WriteData(Setting::Data{dirDialog.GetPath().ToUTF8().data()});
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
  long item = -1;
  while ((item = fileList->GetNextItem(item, wxLIST_NEXT_ALL,
                                       wxLIST_STATE_SELECTED)) != -1) {
    fileList->SetItemState(item, 0, wxLIST_STATE_SELECTED);
  }

  fileList->DeleteAllItems();
  lists = init();

  for (const auto &item : lists) {
    deviceList->Append(item.device_id);
  }

  std::thread([this]() {
    FileManager::local_system_clear();
    wxTheApp->CallAfter(
        [this]() { SetStatusText(wxString::FromUTF8("刷新完成！")); });
  }).detach();
}

void MyFrame::OnImportFromComputer(wxCommandEvent &event) {
  if (deviceList->GetSelection() == wxNOT_FOUND) {
    wxMessageBox(wxString::FromUTF8("请先在设备列表选择一个设备"),
                 wxString::FromUTF8("错误"), wxOK | wxICON_ERROR);
    return;
  }
  const std::string selected_device_id =
      deviceList->GetString(deviceList->GetSelection()).ToStdString();
  wxFileDialog openFileDialog(
      this, wxString::FromUTF8("选择文件"),
      wxFileName::DirName(wxString::FromUTF8(Setting::GetData().Export_Path))
          .GetAbsolutePath(),
      "", wxString::FromUTF8("所有文件 (*.*)|*.*"),
      wxFD_OPEN | wxFD_FILE_MUST_EXIST | wxFD_MULTIPLE);

  if (openFileDialog.ShowModal() == wxID_OK) {
    wxArrayString paths;
    openFileDialog.GetPaths(paths);

    std::thread([=, this]() {
      int success_cnt = 0;
      int total = paths.size();
      for (const auto &path : paths) {
        wxTheApp->CallAfter([=, this]() {
          std::string notify = std::format(
              "正在导入文件:{} 当前进度:{:.1f}%",
              wxString(path.substr(path.rfind("\\") + 1)).ToStdString(),
              static_cast<double>(success_cnt) / total * 100);
          SetStatusText(wxString::FromUTF8(notify));
        });
        if (ADB::PushRemoteFile_Full(
                FileManager::GetListByDeviceID(lists, selected_device_id),
                path.ToStdString())) {
          ++success_cnt;
          fileList->InsertItem(fileList->GetItemCount(),
                               wxString(path.substr(path.rfind("\\") + 1)));
        }
      }
      wxTheApp->CallAfter([=, this]() {
        std::string notify = std::format("导入完成！ {} 成功，{} 失败",
                                         success_cnt, total - success_cnt);
        SetStatusText(wxString::FromUTF8(notify));
      });
    }).detach();
  } else {
    wxMessageBox(wxString::FromUTF8("未选择文件"), wxString::FromUTF8("错误"),
                 wxOK | wxICON_ERROR);
    return;
  }
}

void MyFrame::OnDeviceSelected(wxCommandEvent &event) {
  long item = -1;
  while ((item = fileList->GetNextItem(item, wxLIST_NEXT_ALL,
                                       wxLIST_STATE_SELECTED)) != -1) {
    fileList->SetItemState(item, 0, wxLIST_STATE_SELECTED);
  }

  fileList->DeleteAllItems();
  std::string selected_device_id =
      deviceList->GetString(event.GetSelection()).ToStdString();
  SetStatusText(wxString::FromUTF8("选择设备:" + selected_device_id));
  if (FileManager::Is_Local_Device_MAP_TEMP_Exists(selected_device_id)) {
    std::thread([selected_device_id, this]() {
      auto datas = FileManager::Get_File_List_Json(selected_device_id)
                       .get<std::vector<FileManager::FileData>>();

      int total = datas.size();
      if (total == FileManager::GetListByDeviceID(lists, selected_device_id)
                       .record.size()) {
        wxTheApp->CallAfter([this, datas = std::move(datas), total]() {
          int cnt = 0;
          for (const auto &item : datas) {
            long index = fileList->InsertItem(fileList->GetItemCount(),
                                              wxString::FromUTF8(item.file));
            fileList->SetItem(index, 1, wxString::FromUTF8(item.data.p1));
            fileList->SetItem(index, 2, wxString::FromUTF8(item.data.p2));
            fileList->SetItem(index, 3, wxString::FromUTF8(item.data.statu));
            fileList->SetItem(index, 4, wxString::FromUTF8(item.data.belong));
            std::string notify = std::format(
                "获取到已构建的文件列表，加载中...... 当前进度:{:.1f}%",
                static_cast<double>(++cnt) / total * 100);
            SetStatusText(wxString::FromUTF8(notify));
          }
        });
      } else {
        std::thread([=, this]() {
          auto list = FileManager::GetListByDeviceID(lists, selected_device_id);
          std::map<std::string, std::string> mapping;
          int sum = list.recordlist.size() + list.record.size();
          int cnt = 0;

          std::vector<FileManager::FileData> datas;

          for (const auto &item : list.recordlist) {
            std::string temp_path =
                FileManager::Get_Local_Device_TEMP_Path(list.device_id);
            ADB::PullRemoteFile(list, item, temp_path);
            auto recordlist = hexreader::Get_Record_List(temp_path + item);

            for (const auto &recorditem : recordlist) {
              mapping[recorditem.datetime] = item.substr(item.rfind("_") + 1);
            }

            FileManager::local_system_clear(list.device_id);

            std::string notify =
                std::format("正在构建文件列表中...... 当前进度:{:.1f}%",
                            static_cast<double>(++cnt) / sum * 100);
            wxTheApp->CallAfter(
                [=, this]() { SetStatusText(wxString::FromUTF8(notify)); });
          }

          for (const auto &item : list.record) {
            std::string temp_path =
                FileManager::Get_Local_Device_TEMP_Path(list.device_id);
            ADB::PullRemoteFile(list, item, temp_path);
            auto record = hexreader::Get_Record(temp_path + item);
            FileManager::FileData filedata;
            filedata.file = item;
            int playerIndex = 0;
            for (const auto &playeritem : record.information.inner.players) {
              if (playerIndex == 0)
                filedata.data.p1 = playeritem.userdata.nickname;
              else if (playerIndex == 1)
                filedata.data.p2 = playeritem.userdata.nickname;
              ++playerIndex;
            }

            int result = record.settle_information.inner.statu;
            if (result == 1)
              filedata.data.statu = "平";
            else if (result == 2)
              filedata.data.statu = "负";
            else
              filedata.data.statu = "胜";

            if (mapping.find(item) != mapping.end())
              filedata.data.belong = mapping.at(item);
            else
              filedata.data.belong = "无";

            datas.emplace_back(std::move(filedata));

            std::string notify =
                std::format("正在构建文件列表中...... 当前进度:{:.1f}%",
                            static_cast<double>(++cnt) / sum * 100);
            wxTheApp->CallAfter(
                [=, this]() { SetStatusText(wxString::FromUTF8(notify)); });
          }
          {
            std::ofstream output(
                FileManager::Get_Local_Device_MAP_TEMP(selected_device_id));
            nlohmann::json j = datas;
            output << j.dump(4);
            output.close();
          }
          wxTheApp->CallAfter([this, datas = std::move(datas)]() {
            for (const auto &filedata : datas) {
              long index = fileList->InsertItem(
                  fileList->GetItemCount(), wxString::FromUTF8(filedata.file));
              fileList->SetItem(index, 1, wxString::FromUTF8(filedata.data.p1));
              fileList->SetItem(index, 2, wxString::FromUTF8(filedata.data.p2));
              fileList->SetItem(index, 3,
                                wxString::FromUTF8(filedata.data.statu));
              fileList->SetItem(index, 4,
                                wxString::FromUTF8(filedata.data.belong));
            }
            SetStatusText(wxString::FromUTF8("文件列表构建完毕！"));
          });
        }).detach();
      }
    }).detach();
    return;
  }
  std::thread([=, this]() {
    auto list = FileManager::GetListByDeviceID(lists, selected_device_id);
    std::map<std::string, std::string> mapping;
    int sum = list.recordlist.size() + list.record.size();
    int cnt = 0;

    std::vector<FileManager::FileData> datas;

    for (const auto &item : list.recordlist) {
      std::string temp_path =
          FileManager::Get_Local_Device_TEMP_Path(list.device_id);
      ADB::PullRemoteFile(list, item, temp_path);
      auto recordlist = hexreader::Get_Record_List(temp_path + item);

      for (const auto &recorditem : recordlist) {
        mapping[recorditem.datetime] = item.substr(item.rfind("_") + 1);
      }

      FileManager::local_system_clear(list.device_id);

      std::string notify =
          std::format("正在构建文件列表中...... 当前进度:{:.1f}%",
                      static_cast<double>(++cnt) / sum * 100);
      wxTheApp->CallAfter(
          [=, this]() { SetStatusText(wxString::FromUTF8(notify)); });
    }

    for (const auto &item : list.record) {
      std::string temp_path =
          FileManager::Get_Local_Device_TEMP_Path(list.device_id);
      ADB::PullRemoteFile(list, item, temp_path);
      auto record = hexreader::Get_Record(temp_path + item);
      FileManager::FileData filedata;
      filedata.file = item;
      int playerIndex = 0;
      for (const auto &playeritem : record.information.inner.players) {
        if (playerIndex == 0)
          filedata.data.p1 = playeritem.userdata.nickname;
        else if (playerIndex == 1)
          filedata.data.p2 = playeritem.userdata.nickname;
        ++playerIndex;
      }

      int result = record.settle_information.inner.statu;
      if (result == 1)
        filedata.data.statu = "平";
      else if (result == 2)
        filedata.data.statu = "负";
      else
        filedata.data.statu = "胜";

      if (mapping.find(item) != mapping.end())
        filedata.data.belong = mapping.at(item);
      else
        filedata.data.belong = "无";

      datas.emplace_back(std::move(filedata));

      std::string notify =
          std::format("正在构建文件列表中...... 当前进度:{:.1f}%",
                      static_cast<double>(++cnt) / sum * 100);
      wxTheApp->CallAfter(
          [=, this]() { SetStatusText(wxString::FromUTF8(notify)); });
    }
    {
      std::ofstream output(
          FileManager::Get_Local_Device_MAP_TEMP(selected_device_id));
      nlohmann::json j = datas;
      output << j.dump(4);
      output.close();
    }
    wxTheApp->CallAfter([this, datas = std::move(datas)]() {
      for (const auto &filedata : datas) {
        long index = fileList->InsertItem(fileList->GetItemCount(),
                                          wxString::FromUTF8(filedata.file));
        fileList->SetItem(index, 1, wxString::FromUTF8(filedata.data.p1));
        fileList->SetItem(index, 2, wxString::FromUTF8(filedata.data.p2));
        fileList->SetItem(index, 3, wxString::FromUTF8(filedata.data.statu));
        fileList->SetItem(index, 4, wxString::FromUTF8(filedata.data.belong));
      }
      SetStatusText(wxString::FromUTF8("文件列表构建完毕！"));
    });
  }).detach();
}

void MyFrame::OnFileSelected(wxListEvent &event) {
  int index = event.GetIndex();
  if (index == wxNOT_FOUND)
    return;
  wxString selected_file = fileList->GetItemText(index);
  long state = fileList->GetItemState(index, wxLIST_STATE_SELECTED);
  if (state & wxLIST_STATE_SELECTED) {
    SetStatusText(wxString::FromUTF8("选择文件:" + selected_file));
  } else {
    SetStatusText(wxString::FromUTF8("取消选择:" + selected_file));
  }
}

void MyFrame::OnFileListRightClick(wxMouseEvent &event) {
  wxPoint pos_in_list = event.GetPosition();
  wxPoint pos_on_screen = fileList->ClientToScreen(pos_in_list);
  wxPoint pos_in_frame = this->ScreenToClient(pos_on_screen);

  int flag = 0;
  long file_list_index = fileList->HitTest(pos_in_list, flag);
  if (file_list_index != wxNOT_FOUND) {
    fileList->SetItemState(file_list_index, wxLIST_STATE_SELECTED,
                           wxLIST_STATE_SELECTED);
    SetStatusText(wxString::FromUTF8("选择文件:" +
                                     fileList->GetItemText(file_list_index)));
  }
  std::unique_ptr<wxMenu> menu = std::make_unique<wxMenu>();
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

  PopupMenu(menu.get(), pos_in_frame);
}

void MyFrame::OnExport(wxCommandEvent &event) {
  const std::string selected_device_id =
      deviceList->GetString(deviceList->GetSelection()).ToStdString();

  wxArrayInt selections;
  long item = -1;
  while ((item = fileList->GetNextItem(item, wxLIST_NEXT_ALL,
                                       wxLIST_STATE_SELECTED)) != -1) {
    selections.Add(item);
  }

  std::thread([=, this]() {
    int success_cnt = 0;
    int total = selections.size();

    auto list = FileManager::GetListByDeviceID(lists, selected_device_id);
    std::string export_path = Setting::GetData().Export_Path;
    if (export_path.back() != '\\')
      export_path += '\\';

    for (size_t i = 0; i < selections.size(); ++i) {
      long index = selections[i];
      std::string file = fileList->GetItemText(index, 0).ToStdString();

      auto file_ptr = std::make_shared<std::string>(std::move(file));

      wxTheApp->CallAfter([=, this]() {
        std::string notify =
            std::format("正在导出文件:{} 当前进度:{:.1f}%", *file_ptr,
                        static_cast<double>(success_cnt) / total * 100);
        SetStatusText(wxString::FromUTF8(notify));
      });

      if (FileManager::Copy(list, *file_ptr, utf8_to_gbk(export_path)))
        ++success_cnt;
    }

    wxTheApp->CallAfter([=, this]() {
      std::string notify =
          std::format("导出完成！目录:{} {} 成功，{} 失败", export_path,
                      success_cnt, total - success_cnt);
      SetStatusText(wxString::FromUTF8(notify));
    });
  }).detach();
}

void MyFrame::OnDelete(wxCommandEvent &event) {
  const std::string selected_device_id =
      deviceList->GetString(deviceList->GetSelection()).ToStdString();
  wxArrayInt selections;
  long item = -1;
  while ((item = fileList->GetNextItem(item, wxLIST_NEXT_ALL,
                                       wxLIST_STATE_SELECTED)) != -1) {
    selections.Add(item);
  }
  std::thread([=, this]() {
    int success_cnt = 0;
    int total = selections.size();

    auto list = FileManager::GetListByDeviceID(lists, selected_device_id);
    std::vector<std::string> DelSuccess;

    for (size_t i = 0; i < selections.size(); ++i) {
      long index = selections[i];
      std::string file = fileList->GetItemText(index, 0).ToStdString();

      auto file_ptr = std::make_shared<std::string>(std::move(file));

      wxTheApp->CallAfter([=, this]() {
        std::string notify =
            std::format("正在删除文件:{} 当前进度:{:.1f}%", *file_ptr,
                        static_cast<double>(success_cnt) / total * 100);
        SetStatusText(wxString::FromUTF8(notify));
      });

      if (ADB::DeleteRemoteFile(list, *file_ptr)) {
        DelSuccess.emplace_back(*file_ptr);
        ++success_cnt;
      }
    }
    wxTheApp->CallAfter([=, this]() {
      for (const auto &item : DelSuccess) {
        long itemIndex = -1;
        while ((itemIndex = fileList->GetNextItem(itemIndex)) != -1) {
          wxString filename = fileList->GetItemText(itemIndex, 0);
          if (filename == wxString(item)) {
            fileList->DeleteItem(itemIndex);
            break;
          }
        }
      }
      std::string notify = std::format("删除完成！ {} 成功，{} 失败",
                                       success_cnt, total - success_cnt);
      SetStatusText(wxString::FromUTF8(notify));
    });
  }).detach();
}

void MyFrame::OnSendToDynamicDevice(wxCommandEvent &event) {
  const std::string selected_device_id =
      deviceList->GetString(deviceList->GetSelection()).ToStdString();
  wxArrayInt selections;
  long item = -1;
  while ((item = fileList->GetNextItem(item, wxLIST_NEXT_ALL,
                                       wxLIST_STATE_SELECTED)) != -1) {
    selections.Add(item);
  }

  std::thread([=, this]() {
    int success_cnt = 0;
    int total = selections.size();

    auto list = FileManager::GetListByDeviceID(lists, selected_device_id);
    std::string export_path = Setting::GetData().Export_Path;

    for (size_t i = 0; i < selections.size(); ++i) {
      long index = selections[i];
      std::string file = fileList->GetItemText(index, 0).ToStdString();

      auto file_ptr = std::make_shared<std::string>(std::move(file));

      wxTheApp->CallAfter([=, this]() {
        std::string notify =
            std::format("正在发送文件:{} 当前进度:{:.1f}%", *file_ptr,
                        static_cast<double>(success_cnt) / total * 100);
        SetStatusText(wxString::FromUTF8(notify));
      });
      if (ADB::PushRemoteFile(lists[event.GetId() - 2000], list.device_id,
                              *file_ptr))
        ++success_cnt;
    }

    wxTheApp->CallAfter([=, this]() {
      std::string notify = std::format("发送完成！ {} 成功，{} 失败",
                                       success_cnt, total - success_cnt);
      SetStatusText(wxString::FromUTF8(notify));
    });
  }).detach();
}

void MyFrame::OnClose(wxCloseEvent &event) {
  FileManager::local_system_clear();
  RunCommand("taskkill /f /im adb.exe");
  Destroy();
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