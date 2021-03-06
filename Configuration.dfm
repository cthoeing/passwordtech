object ConfigurationDlg: TConfigurationDlg
  Left = 167
  Top = 187
  BorderIcons = [biSystemMenu, biMaximize]
  Caption = 'Configuration'
  ClientHeight = 424
  ClientWidth = 429
  Color = clBtnFace
  Constraints.MinHeight = 421
  Constraints.MinWidth = 445
  Font.Charset = ANSI_CHARSET
  Font.Color = clWindowText
  Font.Height = -11
  Font.Name = 'Tahoma'
  Font.Style = []
  OldCreateOrder = False
  PopupMode = pmAuto
  OnShow = FormShow
  DesignSize = (
    429
    424)
  PixelsPerInch = 96
  TextHeight = 13
  object ConfigPages: TPageControl
    Left = 11
    Top = 8
    Width = 410
    Height = 369
    ActivePage = UpdatesSheet
    Anchors = [akLeft, akTop, akRight, akBottom]
    HotTrack = True
    TabOrder = 0
    object GeneralSheet: TTabSheet
      Caption = 'General'
      DesignSize = (
        402
        341)
      object ChangeFontLbl: TLabel
        Left = 8
        Top = 16
        Width = 162
        Height = 13
        Caption = 'Change font for the GUI controls:'
      end
      object FontSampleLbl: TLabel
        Left = 152
        Top = 42
        Width = 21
        Height = 13
        Caption = 'Test'
      end
      object AutotypeDelayLbl: TLabel
        Left = 8
        Top = 154
        Width = 201
        Height = 13
        Caption = 'Autotype delay between characters (ms):'
      end
      object SelectFontBtn: TButton
        Left = 8
        Top = 36
        Width = 137
        Height = 25
        Caption = 'Select font'
        TabOrder = 0
        OnClick = SelectFontBtnClick
      end
      object ShowSysTrayIconConstCheck: TCheckBox
        Left = 8
        Top = 77
        Width = 385
        Height = 17
        Caption = 'Show system tray icon constantly'
        TabOrder = 1
      end
      object MinimizeToSysTrayCheck: TCheckBox
        Left = 8
        Top = 100
        Width = 385
        Height = 17
        Caption = 'Minimize program to system tray'
        TabOrder = 2
      end
      object AutotypeDelayBox: TEdit
        Left = 266
        Top = 151
        Width = 48
        Height = 21
        Anchors = [akTop, akRight]
        TabOrder = 4
        Text = '0'
      end
      object AutotypeDelayUpDown: TUpDown
        Left = 314
        Top = 151
        Width = 16
        Height = 21
        Anchors = [akTop, akRight]
        Associate = AutotypeDelayBox
        Max = 1000
        TabOrder = 5
      end
      object MinimizeAutotypeCheck: TCheckBox
        Left = 8
        Top = 123
        Width = 385
        Height = 17
        Caption = 'Minimize before performing autotype'
        TabOrder = 3
      end
      object AskBeforeExitCheck: TCheckBox
        Left = 8
        Top = 178
        Width = 380
        Height = 17
        Anchors = [akLeft, akTop, akRight]
        Caption = 'Ask before exiting application'
        TabOrder = 6
      end
      object LaunchSystemStartupCheck: TCheckBox
        Left = 8
        Top = 201
        Width = 380
        Height = 17
        Caption = 'Launch application on system startup (for current user)'
        TabOrder = 7
      end
    end
    object SecuritySheet: TTabSheet
      Caption = 'Security'
      ImageIndex = 6
      DesignSize = (
        402
        341)
      object RandomPoolCipherLbl: TLabel
        Left = 8
        Top = 16
        Width = 317
        Height = 13
        Caption = 'Encryption algorithm for generating random data via random pool:'
      end
      object AutoClearClipCheck: TCheckBox
        Left = 8
        Top = 126
        Width = 386
        Height = 17
        Anchors = [akLeft, akTop, akRight]
        Caption = 
          'Clear clipboard automatically after the following time (seconds)' +
          ':'
        TabOrder = 2
        OnClick = AutoClearClipCheckClick
      end
      object AutoClearClipTimeBox: TEdit
        Left = 53
        Top = 149
        Width = 49
        Height = 21
        TabOrder = 3
        Text = '1'
      end
      object AutoClearClipTimeSpinBtn: TUpDown
        Left = 102
        Top = 149
        Width = 16
        Height = 21
        Associate = AutoClearClipTimeBox
        Min = 1
        Max = 32767
        Position = 1
        TabOrder = 4
      end
      object TestCommonPasswCheck: TCheckBox
        Left = 8
        Top = 103
        Width = 386
        Height = 17
        Anchors = [akLeft, akTop, akRight]
        Caption = 'Test passwords against list of common passwords'
        TabOrder = 1
      end
      object RandomPoolCipherList: TComboBox
        Left = 8
        Top = 35
        Width = 377
        Height = 21
        Style = csDropDownList
        Anchors = [akLeft, akTop, akRight]
        TabOrder = 0
      end
      object AutoClearPasswCheck: TCheckBox
        Left = 8
        Top = 176
        Width = 386
        Height = 17
        Anchors = [akLeft, akTop, akRight]
        Caption = 
          'Clear password box automatically after the following time (secon' +
          'ds):'
        TabOrder = 5
        OnClick = AutoClearPasswCheckClick
      end
      object AutoClearPasswTimeSpinBtn: TUpDown
        Left = 102
        Top = 199
        Width = 16
        Height = 21
        Associate = AutoClearPasswTimeBox
        Min = 1
        Max = 32767
        Position = 1
        TabOrder = 7
      end
      object AutoClearPasswTimeBox: TEdit
        Left = 53
        Top = 199
        Width = 49
        Height = 21
        TabOrder = 6
        Text = '1'
      end
      object BenchmarkBtn: TButton
        Left = 232
        Top = 62
        Width = 153
        Height = 25
        Anchors = [akTop, akRight]
        Caption = 'Benchmark...'
        TabOrder = 8
        OnClick = BenchmarkBtnClick
      end
    end
    object HotKeySheet: TTabSheet
      Caption = 'Hot Keys'
      DesignSize = (
        402
        341)
      object HotKeyLbl: TLabel
        Left = 8
        Top = 108
        Width = 91
        Height = 13
        Caption = 'Shortcut to assign:'
      end
      object HotKeyActionsGroup: TGroupBox
        Left = 8
        Top = 16
        Width = 386
        Height = 77
        Anchors = [akLeft, akTop, akRight]
        Caption = 'Actions'
        TabOrder = 0
        DesignSize = (
          386
          77)
        object HotKeyShowMainWinCheck: TCheckBox
          Left = 8
          Top = 20
          Width = 281
          Height = 17
          Caption = 'Show/restore main window'
          TabOrder = 0
        end
        object HotKeyActionsList: TComboBox
          Left = 8
          Top = 44
          Width = 370
          Height = 21
          Style = csDropDownList
          Anchors = [akLeft, akTop, akRight]
          DropDownCount = 10
          TabOrder = 1
          Items.Strings = (
            'No further action'
            'Generate single password in main window'
            'Generate multiple passwords'
            'Generate password and copy it to the clipboard'
            'Generate password and show it in a message box'
            'Generate password and perform autotype'
            'Show MP password generator'
            'Show password manager'
            'Search database for keyword in window title'
            'Search database for keyword and perform autotype')
        end
      end
      object HotKeyView: TListView
        Left = 8
        Top = 136
        Width = 385
        Height = 162
        Anchors = [akLeft, akTop, akRight, akBottom]
        Columns = <
          item
            Caption = 'Shortcut'
            Width = 100
          end
          item
            Caption = 'Action'
            Width = 250
          end>
        MultiSelect = True
        RowSelect = True
        TabOrder = 3
        ViewStyle = vsReport
        OnSelectItem = HotKeyViewSelectItem
      end
      object HotKeyBox: THotKey
        Left = 160
        Top = 104
        Width = 146
        Height = 21
        Anchors = [akLeft, akTop, akRight]
        HotKey = 0
        InvalidKeys = [hcNone]
        Modifiers = []
        TabOrder = 1
        OnChange = HotKeyBoxChange
      end
      object AddBtn: TButton
        Left = 312
        Top = 102
        Width = 81
        Height = 25
        Anchors = [akTop, akRight]
        Caption = 'Add'
        Enabled = False
        TabOrder = 2
        OnClick = AddBtnClick
      end
      object RemoveBtn: TButton
        Left = 312
        Top = 305
        Width = 81
        Height = 25
        Anchors = [akRight, akBottom]
        Caption = 'Remove'
        Enabled = False
        TabOrder = 4
        OnClick = RemoveBtnClick
      end
    end
    object FilesSheet: TTabSheet
      Caption = 'Files'
      object FileEncodingLbl: TLabel
        Left = 8
        Top = 16
        Width = 156
        Height = 13
        Caption = 'Character encoding of text files:'
      end
      object NewlineCharLbl: TLabel
        Left = 8
        Top = 72
        Width = 139
        Height = 13
        Caption = 'Newline character sequence:'
      end
      object FileEncodingList: TComboBox
        Left = 8
        Top = 35
        Width = 154
        Height = 21
        Style = csDropDownList
        ItemIndex = 0
        TabOrder = 0
        Text = 'ANSI'
        Items.Strings = (
          'ANSI'
          'UTF-16'
          'UTF-16 Big Endian'
          'UTF-8')
      end
      object NewlineCharList: TComboBox
        Left = 8
        Top = 91
        Width = 154
        Height = 21
        Style = csDropDownList
        ItemIndex = 0
        TabOrder = 1
        Text = 'Windows (\r\n)'
        Items.Strings = (
          'Windows (\r\n)'
          'Unix (\n)')
      end
    end
    object UpdatesSheet: TTabSheet
      Caption = 'Updates'
      object AutoCheckUpdatesLbl: TLabel
        Left = 8
        Top = 16
        Width = 141
        Height = 13
        Caption = 'Automatic check for updates:'
      end
      object AutoCheckUpdatesList: TComboBox
        Left = 8
        Top = 35
        Width = 154
        Height = 21
        Style = csDropDownList
        TabOrder = 0
        Items.Strings = (
          'Daily'
          'Weekly'
          'Monthly'
          'Disabled')
      end
    end
    object LanguageSheet: TTabSheet
      Caption = 'Language'
      object SelectLanguageLbl: TLabel
        Left = 8
        Top = 16
        Width = 80
        Height = 13
        Caption = 'Select language:'
      end
      object LanguageList: TComboBox
        Left = 8
        Top = 35
        Width = 249
        Height = 21
        Style = csDropDownList
        TabOrder = 0
      end
    end
    object DatabaseSheet: TTabSheet
      Caption = 'Database'
      ImageIndex = 5
      DesignSize = (
        402
        341)
      object DefaultAutotypeSeqLbl: TLabel
        Left = 8
        Top = 296
        Width = 135
        Height = 13
        Caption = 'Default autotype sequence:'
      end
      object LockMinimizeCheck: TCheckBox
        Left = 8
        Top = 63
        Width = 377
        Height = 17
        Anchors = [akLeft, akTop, akRight]
        Caption = 'Lock database when minimizing application or database window'
        TabOrder = 2
      end
      object LockIdleCheck: TCheckBox
        Left = 8
        Top = 86
        Width = 377
        Height = 17
        Anchors = [akLeft, akTop, akRight]
        Caption = 'Lock database after the following idle time (seconds):'
        TabOrder = 3
        OnClick = LockIdleCheckClick
      end
      object LockIdleTimeBox: TEdit
        Left = 54
        Top = 109
        Width = 51
        Height = 21
        TabOrder = 4
        Text = '60'
      end
      object LockIdleTimeUpDown: TUpDown
        Left = 105
        Top = 109
        Width = 16
        Height = 21
        Associate = LockIdleTimeBox
        Min = 10
        Max = 32767
        Position = 60
        TabOrder = 5
      end
      object CreateBackupCheck: TCheckBox
        Left = 8
        Top = 136
        Width = 377
        Height = 17
        Anchors = [akLeft, akTop, akRight]
        Caption = 'Create backup of database before saving'
        TabOrder = 7
        OnClick = CreateBackupCheckClick
      end
      object MaxNumBackupsBox: TEdit
        Left = 282
        Top = 159
        Width = 51
        Height = 21
        Anchors = [akTop, akRight]
        NumbersOnly = True
        TabOrder = 9
        Text = '1'
      end
      object MaxNumBackupsUpDown: TUpDown
        Left = 333
        Top = 159
        Width = 16
        Height = 21
        Anchors = [akTop, akRight]
        Associate = MaxNumBackupsBox
        Min = 1
        Max = 999
        Position = 1
        TabOrder = 10
      end
      object OpenDbOnStartupCheck: TCheckBox
        Left = 8
        Top = 210
        Width = 377
        Height = 17
        Caption = 'Open last used database on startup'
        TabOrder = 12
      end
      object ClearClipMinimizeCheck: TCheckBox
        Left = 8
        Top = 16
        Width = 380
        Height = 17
        Caption = 'Clear clipboard on minimize'
        TabOrder = 0
      end
      object ClearClipExitCheck: TCheckBox
        Left = 8
        Top = 39
        Width = 380
        Height = 17
        Caption = 'Clear clipboard on exit'
        TabOrder = 1
      end
      object OpenWindowOnStartupCheck: TCheckBox
        Left = 8
        Top = 187
        Width = 377
        Height = 17
        Caption = 'Open window on startup'
        TabOrder = 11
      end
      object LockAutoSaveCheck: TCheckBox
        Left = 136
        Top = 113
        Width = 249
        Height = 17
        Caption = 'Save automatically'
        TabOrder = 6
      end
      object DefaultAutotypeSeqBox: TEdit
        Left = 176
        Top = 293
        Width = 212
        Height = 21
        Anchors = [akLeft, akTop, akRight]
        TabOrder = 16
      end
      object NumberBackupsCheck: TCheckBox
        Left = 54
        Top = 159
        Width = 222
        Height = 17
        Anchors = [akLeft, akTop, akRight]
        Caption = 'Number backups consecutively - up to:'
        TabOrder = 8
        OnClick = NumberBackupsCheckClick
      end
      object AutoSaveCheck: TCheckBox
        Left = 8
        Top = 257
        Width = 162
        Height = 21
        Caption = 'Save automatically:'
        TabOrder = 14
        OnClick = AutoSaveCheckClick
      end
      object AutoSaveList: TComboBox
        Left = 176
        Top = 257
        Width = 209
        Height = 21
        Style = csDropDownList
        Anchors = [akLeft, akTop, akRight]
        ItemIndex = 0
        TabOrder = 15
        Text = 'After adding/modifying an entry'
        Items.Strings = (
          'After adding/modifying an entry'
          'After every change')
      end
      object WarnExpiredEntriesCheck: TCheckBox
        Left = 8
        Top = 234
        Width = 377
        Height = 17
        Caption = 'Warn if database contains expired entries'
        TabOrder = 13
      end
    end
  end
  object OKBtn: TButton
    Left = 265
    Top = 388
    Width = 75
    Height = 25
    Anchors = [akRight, akBottom]
    Caption = 'OK'
    Default = True
    TabOrder = 1
    OnClick = OKBtnClick
  end
  object CancelBtn: TButton
    Left = 345
    Top = 388
    Width = 75
    Height = 25
    Anchors = [akRight, akBottom]
    Cancel = True
    Caption = 'Cancel'
    ModalResult = 2
    TabOrder = 2
  end
  object FontDlg: TFontDialog
    Font.Charset = DEFAULT_CHARSET
    Font.Color = clWindowText
    Font.Height = -11
    Font.Name = 'MS Sans Serif'
    Font.Style = []
    Left = 8
    Top = 344
  end
end
