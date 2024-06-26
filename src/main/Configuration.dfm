object ConfigurationDlg: TConfigurationDlg
  Left = 167
  Top = 187
  BorderIcons = [biSystemMenu, biMaximize]
  Caption = 'Configuration'
  ClientHeight = 517
  ClientWidth = 552
  Color = clBtnFace
  Constraints.MinHeight = 456
  Constraints.MinWidth = 443
  Font.Charset = ANSI_CHARSET
  Font.Color = clWindowText
  Font.Height = -14
  Font.Name = 'Tahoma'
  Font.Style = []
  PopupMode = pmAuto
  OnShow = FormShow
  PixelsPerInch = 120
  TextHeight = 17
  object ConfigPages: TPageControl
    Tag = 15
    Left = 10
    Top = 10
    Width = 528
    Height = 452
    Margins.Left = 4
    Margins.Top = 4
    Margins.Right = 4
    Margins.Bottom = 4
    ActivePage = GeneralSheet
    Constraints.MinWidth = 490
    HotTrack = True
    TabOrder = 0
    object GeneralSheet: TTabSheet
      Margins.Left = 4
      Margins.Top = 4
      Margins.Right = 4
      Margins.Bottom = 4
      Caption = 'General'
      DesignSize = (
        520
        420)
      object ChangeFontLbl: TLabel
        Left = 10
        Top = 90
        Width = 205
        Height = 17
        Margins.Left = 4
        Margins.Top = 4
        Margins.Right = 4
        Margins.Bottom = 4
        Caption = 'Change font for the GUI controls:'
      end
      object FontSampleLbl: TLabel
        Left = 203
        Top = 120
        Width = 26
        Height = 17
        Margins.Left = 4
        Margins.Top = 4
        Margins.Right = 4
        Margins.Bottom = 4
        Caption = 'Test'
      end
      object AutotypeDelayLbl: TLabel
        Left = 10
        Top = 263
        Width = 254
        Height = 17
        Margins.Left = 4
        Margins.Top = 4
        Margins.Right = 4
        Margins.Bottom = 4
        Caption = 'Autotype delay between characters (ms):'
      end
      object UiStyleLbl: TLabel
        Left = 10
        Top = 20
        Width = 120
        Height = 17
        Margins.Left = 4
        Margins.Top = 4
        Margins.Right = 4
        Margins.Bottom = 4
        Caption = 'User interface style:'
      end
      object AppIconLbl: TLabel
        Left = 280
        Top = 20
        Width = 99
        Height = 17
        Margins.Left = 4
        Margins.Top = 4
        Margins.Right = 4
        Margins.Bottom = 4
        Caption = 'Application icon:'
      end
      object SelectFontBtn: TButton
        Left = 10
        Top = 114
        Width = 171
        Height = 31
        Margins.Left = 4
        Margins.Top = 4
        Margins.Right = 4
        Margins.Bottom = 4
        Caption = 'Select font'
        DropDownMenu = SelectFontMenu
        Style = bsSplitButton
        TabOrder = 2
        OnClick = SelectFontBtnClick
      end
      object ShowSysTrayIconConstCheck: TCheckBox
        Left = 10
        Top = 166
        Width = 481
        Height = 22
        Margins.Left = 4
        Margins.Top = 4
        Margins.Right = 4
        Margins.Bottom = 4
        Caption = 'Show system tray icon constantly'
        TabOrder = 3
      end
      object MinimizeToSysTrayCheck: TCheckBox
        Left = 10
        Top = 195
        Width = 481
        Height = 21
        Margins.Left = 4
        Margins.Top = 4
        Margins.Right = 4
        Margins.Bottom = 4
        Caption = 'Minimize program to system tray'
        TabOrder = 4
      end
      object AutotypeDelayBox: TEdit
        Left = 326
        Top = 259
        Width = 60
        Height = 25
        Margins.Left = 4
        Margins.Top = 4
        Margins.Right = 4
        Margins.Bottom = 4
        AutoSelect = False
        TabOrder = 6
        Text = '0'
      end
      object AutotypeDelaySpinBtn: TUpDown
        Left = 386
        Top = 259
        Width = 20
        Height = 25
        Margins.Left = 4
        Margins.Top = 4
        Margins.Right = 4
        Margins.Bottom = 4
        Associate = AutotypeDelayBox
        Max = 1000
        TabOrder = 7
      end
      object MinimizeAutotypeCheck: TCheckBox
        Left = 10
        Top = 224
        Width = 481
        Height = 21
        Margins.Left = 4
        Margins.Top = 4
        Margins.Right = 4
        Margins.Bottom = 4
        Caption = 'Minimize before performing autotype'
        TabOrder = 5
      end
      object AskBeforeExitCheck: TCheckBox
        Left = 10
        Top = 293
        Width = 492
        Height = 21
        Margins.Left = 4
        Margins.Top = 4
        Margins.Right = 4
        Margins.Bottom = 4
        Anchors = [akLeft, akTop, akRight]
        Caption = 'Ask before exiting application'
        TabOrder = 8
      end
      object LaunchSystemStartupCheck: TCheckBox
        Left = 10
        Top = 350
        Width = 475
        Height = 21
        Margins.Left = 4
        Margins.Top = 4
        Margins.Right = 4
        Margins.Bottom = 4
        Caption = 'Launch application on system startup (for current user)'
        TabOrder = 11
      end
      object UiStylesList: TComboBox
        Left = 10
        Top = 44
        Width = 241
        Height = 25
        Margins.Left = 4
        Margins.Top = 4
        Margins.Right = 4
        Margins.Bottom = 4
        Style = csDropDownList
        Sorted = True
        TabOrder = 0
      end
      object LoadProfileStartupCheck: TCheckBox
        Left = 10
        Top = 321
        Width = 300
        Height = 22
        Margins.Left = 4
        Margins.Top = 4
        Margins.Right = 4
        Margins.Bottom = 4
        Caption = 'Load the following profile on startup:'
        TabOrder = 9
        OnClick = LoadProfileStartupCheckClick
      end
      object LoadProfileBox: TComboBox
        Left = 326
        Top = 316
        Width = 176
        Height = 25
        Margins.Left = 4
        Margins.Top = 4
        Margins.Right = 4
        Margins.Bottom = 4
        Style = csDropDownList
        Anchors = [akLeft, akTop, akRight]
        TabOrder = 10
      end
      object AppIconList: TComboBoxEx
        Left = 280
        Top = 44
        Width = 222
        Height = 26
        Margins.Left = 4
        Margins.Top = 4
        Margins.Right = 4
        Margins.Bottom = 4
        ItemsEx = <>
        Style = csExDropDownList
        TabOrder = 1
        Images = AppIconImageList
      end
    end
    object SecuritySheet: TTabSheet
      Margins.Left = 4
      Margins.Top = 4
      Margins.Right = 4
      Margins.Bottom = 4
      Caption = 'Security'
      ImageIndex = 6
      DesignSize = (
        520
        420)
      object RandomPoolCipherLbl: TLabel
        Left = 10
        Top = 20
        Width = 409
        Height = 17
        Margins.Left = 4
        Margins.Top = 4
        Margins.Right = 4
        Margins.Bottom = 4
        Caption = 'Encryption algorithm for generating random data via random pool:'
      end
      object AutoClearClipCheck: TCheckBox
        Left = 10
        Top = 186
        Width = 500
        Height = 22
        Margins.Left = 4
        Margins.Top = 4
        Margins.Right = 4
        Margins.Bottom = 4
        Anchors = [akLeft, akTop, akRight]
        Caption = 
          'Clear clipboard automatically after the following time (seconds)' +
          ':'
        TabOrder = 5
        OnClick = AutoClearClipCheckClick
      end
      object AutoClearClipTimeBox: TEdit
        Left = 60
        Top = 215
        Width = 61
        Height = 25
        Margins.Left = 4
        Margins.Top = 4
        Margins.Right = 4
        Margins.Bottom = 4
        TabOrder = 6
        Text = '1'
      end
      object AutoClearClipTimeSpinBtn: TUpDown
        Left = 121
        Top = 215
        Width = 20
        Height = 25
        Margins.Left = 4
        Margins.Top = 4
        Margins.Right = 4
        Margins.Bottom = 4
        Associate = AutoClearClipTimeBox
        Min = 1
        Max = 32767
        Position = 1
        TabOrder = 7
      end
      object TestCommonPasswCheck: TCheckBox
        Left = 10
        Top = 129
        Width = 500
        Height = 21
        Margins.Left = 4
        Margins.Top = 4
        Margins.Right = 4
        Margins.Bottom = 4
        Anchors = [akLeft, akTop, akRight]
        Caption = 'Test passwords against list of common passwords'
        TabOrder = 3
      end
      object RandomPoolCipherList: TComboBox
        Left = 10
        Top = 44
        Width = 488
        Height = 25
        Margins.Left = 4
        Margins.Top = 4
        Margins.Right = 4
        Margins.Bottom = 4
        Style = csDropDownList
        Anchors = [akLeft, akTop, akRight]
        TabOrder = 0
      end
      object AutoClearPasswCheck: TCheckBox
        Left = 10
        Top = 249
        Width = 500
        Height = 21
        Margins.Left = 4
        Margins.Top = 4
        Margins.Right = 4
        Margins.Bottom = 4
        Anchors = [akLeft, akTop, akRight]
        Caption = 
          'Clear password box automatically after the following time (secon' +
          'ds):'
        TabOrder = 8
        OnClick = AutoClearPasswCheckClick
      end
      object AutoClearPasswTimeSpinBtn: TUpDown
        Left = 121
        Top = 278
        Width = 20
        Height = 25
        Margins.Left = 4
        Margins.Top = 4
        Margins.Right = 4
        Margins.Bottom = 4
        Associate = AutoClearPasswTimeBox
        Min = 1
        Max = 32767
        Position = 1
        TabOrder = 10
      end
      object AutoClearPasswTimeBox: TEdit
        Left = 60
        Top = 278
        Width = 61
        Height = 25
        Margins.Left = 4
        Margins.Top = 4
        Margins.Right = 4
        Margins.Bottom = 4
        TabOrder = 9
        Text = '1'
      end
      object BenchmarkBtn: TButton
        Left = 182
        Top = 78
        Width = 191
        Height = 31
        Margins.Left = 4
        Margins.Top = 4
        Margins.Right = 4
        Margins.Bottom = 4
        Anchors = [akTop, akRight]
        Caption = 'Benchmark...'
        TabOrder = 1
        OnClick = BenchmarkBtnClick
      end
      object UseAdvancedPasswEst: TCheckBox
        Left = 10
        Top = 158
        Width = 471
        Height = 21
        Margins.Left = 4
        Margins.Top = 4
        Margins.Right = 4
        Margins.Bottom = 4
        Caption = 'Use advanced password strength estimation (zxcvbn)'
        TabOrder = 4
      end
      object BenchmarkMemList: TComboBox
        Left = 381
        Top = 78
        Width = 117
        Height = 25
        Hint = 'Data size for benchmark test'
        Margins.Left = 4
        Margins.Top = 4
        Margins.Right = 4
        Margins.Bottom = 4
        Style = csDropDownList
        Anchors = [akTop, akRight]
        ParentShowHint = False
        ShowHint = True
        TabOrder = 2
      end
    end
    object HotKeySheet: TTabSheet
      Margins.Left = 4
      Margins.Top = 4
      Margins.Right = 4
      Margins.Bottom = 4
      Caption = 'Hot Keys'
      DesignSize = (
        520
        420)
      object HotKeyLbl: TLabel
        Left = 10
        Top = 134
        Width = 117
        Height = 17
        Margins.Left = 4
        Margins.Top = 4
        Margins.Right = 4
        Margins.Bottom = 4
        Caption = 'Shortcut to assign:'
      end
      object HotKeyActionsGroup: TGroupBox
        Left = 10
        Top = 20
        Width = 500
        Height = 96
        Margins.Left = 4
        Margins.Top = 4
        Margins.Right = 4
        Margins.Bottom = 4
        Anchors = [akLeft, akTop, akRight]
        Caption = 'Actions'
        TabOrder = 0
        DesignSize = (
          500
          96)
        object HotKeyShowMainWinCheck: TCheckBox
          Left = 10
          Top = 25
          Width = 351
          Height = 21
          Margins.Left = 4
          Margins.Top = 4
          Margins.Right = 4
          Margins.Bottom = 4
          Caption = 'Show/restore main window'
          TabOrder = 0
        end
        object HotKeyActionsList: TComboBox
          Left = 10
          Top = 55
          Width = 480
          Height = 25
          Margins.Left = 4
          Margins.Top = 4
          Margins.Right = 4
          Margins.Bottom = 4
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
        Left = 10
        Top = 170
        Width = 498
        Height = 196
        Margins.Left = 4
        Margins.Top = 4
        Margins.Right = 4
        Margins.Bottom = 4
        Anchors = [akLeft, akTop, akRight, akBottom]
        Columns = <
          item
            Caption = 'Shortcut'
            Width = 125
          end
          item
            Caption = 'Action'
            Width = 313
          end>
        MultiSelect = True
        ReadOnly = True
        RowSelect = True
        TabOrder = 3
        ViewStyle = vsReport
        OnSelectItem = HotKeyViewSelectItem
      end
      object HotKeyBox: THotKey
        Left = 200
        Top = 130
        Width = 200
        Height = 25
        Margins.Left = 4
        Margins.Top = 4
        Margins.Right = 4
        Margins.Bottom = 4
        Anchors = [akLeft, akTop, akRight]
        HotKey = 0
        InvalidKeys = [hcNone]
        Modifiers = []
        TabOrder = 1
        OnChange = HotKeyBoxChange
      end
      object AddBtn: TButton
        Left = 407
        Top = 128
        Width = 101
        Height = 31
        Margins.Left = 4
        Margins.Top = 4
        Margins.Right = 4
        Margins.Bottom = 4
        Anchors = [akTop, akRight]
        Caption = 'Add'
        Enabled = False
        TabOrder = 2
        OnClick = AddBtnClick
      end
      object RemoveBtn: TButton
        Left = 407
        Top = 375
        Width = 101
        Height = 31
        Margins.Left = 4
        Margins.Top = 4
        Margins.Right = 4
        Margins.Bottom = 4
        Anchors = [akRight, akBottom]
        Caption = 'Remove'
        Enabled = False
        TabOrder = 4
        OnClick = RemoveBtnClick
      end
    end
    object FilesSheet: TTabSheet
      Margins.Left = 4
      Margins.Top = 4
      Margins.Right = 4
      Margins.Bottom = 4
      Caption = 'Files'
      DesignSize = (
        520
        420)
      object CharEncodingGroup: TRadioGroup
        Left = 10
        Top = 20
        Width = 498
        Height = 141
        Margins.Left = 4
        Margins.Top = 4
        Margins.Right = 4
        Margins.Bottom = 4
        Anchors = [akLeft, akTop, akRight]
        Caption = 'Character encoding of text files'
        Items.Strings = (
          'ANSI'
          'UTF-16'
          'UTF-16 Big Endian'
          'UTF-8')
        TabOrder = 0
      end
      object NewlineCharGroup: TRadioGroup
        Left = 10
        Top = 169
        Width = 498
        Height = 85
        Margins.Left = 4
        Margins.Top = 4
        Margins.Right = 4
        Margins.Bottom = 4
        Anchors = [akLeft, akTop, akRight]
        Caption = 'Newline character sequence'
        Items.Strings = (
          'Windows (\r\n)'
          'Unix (\n)')
        TabOrder = 1
      end
    end
    object UpdatesSheet: TTabSheet
      Margins.Left = 4
      Margins.Top = 4
      Margins.Right = 4
      Margins.Bottom = 4
      Caption = 'Updates'
      DesignSize = (
        520
        420)
      object UpdateCheckGroup: TRadioGroup
        Left = 10
        Top = 20
        Width = 498
        Height = 141
        Margins.Left = 4
        Margins.Top = 4
        Margins.Right = 4
        Margins.Bottom = 4
        Anchors = [akLeft, akTop, akRight]
        Caption = 'Automatic check for updates'
        Items.Strings = (
          'Daily'
          'Weekly'
          'Monthly'
          'Disabled')
        TabOrder = 0
      end
    end
    object LanguageSheet: TTabSheet
      Margins.Left = 4
      Margins.Top = 4
      Margins.Right = 4
      Margins.Bottom = 4
      Caption = 'Language'
      object SelectLanguageLbl: TLabel
        Left = 10
        Top = 20
        Width = 100
        Height = 17
        Margins.Left = 4
        Margins.Top = 4
        Margins.Right = 4
        Margins.Bottom = 4
        Caption = 'Select language:'
      end
      object LanguageList: TComboBox
        Left = 10
        Top = 44
        Width = 311
        Height = 25
        Margins.Left = 4
        Margins.Top = 4
        Margins.Right = 4
        Margins.Bottom = 4
        Style = csDropDownList
        TabOrder = 0
        OnSelect = LanguageListSelect
      end
      object ConvertLangFileBtn: TButton
        Left = 220
        Top = 77
        Width = 248
        Height = 31
        Margins.Left = 4
        Margins.Top = 4
        Margins.Right = 4
        Margins.Bottom = 4
        Caption = 'Convert to new PO file format...'
        Enabled = False
        TabOrder = 3
        OnClick = ConvertLangFileBtnClick
      end
      object InstallLanguageBtn: TButton
        Left = 10
        Top = 77
        Width = 100
        Height = 31
        Margins.Left = 4
        Margins.Top = 4
        Margins.Right = 4
        Margins.Bottom = 4
        Caption = 'Install...'
        TabOrder = 1
        OnClick = InstallLanguageBtnClick
      end
      object RemoveLanguageBtn: TButton
        Left = 118
        Top = 77
        Width = 94
        Height = 31
        Margins.Left = 4
        Margins.Top = 4
        Margins.Right = 4
        Margins.Bottom = 4
        Caption = 'Remove'
        TabOrder = 2
        OnClick = RemoveLanguageBtnClick
      end
    end
    object DatabaseSheet: TTabSheet
      Margins.Left = 4
      Margins.Top = 4
      Margins.Right = 4
      Margins.Bottom = 4
      Caption = 'Database'
      ImageIndex = 5
      DesignSize = (
        520
        420)
      object DefaultAutotypeSeqLbl: TLabel
        Left = 10
        Top = 391
        Width = 170
        Height = 17
        Margins.Left = 4
        Margins.Top = 4
        Margins.Right = 4
        Margins.Bottom = 4
        Caption = 'Default autotype sequence:'
      end
      object WarnExpireNumDaysLbl: TLabel
        Left = 73
        Top = 324
        Width = 103
        Height = 17
        Margins.Left = 4
        Margins.Top = 4
        Margins.Right = 4
        Margins.Bottom = 4
        Caption = 'Number of days:'
      end
      object LockMinimizeCheck: TCheckBox
        Left = 10
        Top = 49
        Width = 488
        Height = 21
        Margins.Left = 4
        Margins.Top = 4
        Margins.Right = 4
        Margins.Bottom = 4
        Anchors = [akLeft, akTop, akRight]
        Caption = 'Lock database when minimizing application or database window'
        TabOrder = 1
      end
      object LockIdleCheck: TCheckBox
        Left = 10
        Top = 78
        Width = 488
        Height = 21
        Margins.Left = 4
        Margins.Top = 4
        Margins.Right = 4
        Margins.Bottom = 4
        Anchors = [akLeft, akTop, akRight]
        Caption = 'Lock database after the following idle time (seconds):'
        TabOrder = 2
        OnClick = LockIdleCheckClick
      end
      object LockIdleTimeBox: TEdit
        Left = 73
        Top = 106
        Width = 63
        Height = 25
        Margins.Left = 4
        Margins.Top = 4
        Margins.Right = 4
        Margins.Bottom = 4
        TabOrder = 3
        Text = '60'
      end
      object LockIdleTimeSpinBtn: TUpDown
        Left = 136
        Top = 106
        Width = 20
        Height = 25
        Margins.Left = 4
        Margins.Top = 4
        Margins.Right = 4
        Margins.Bottom = 4
        Associate = LockIdleTimeBox
        Min = 10
        Max = 32767
        Position = 60
        TabOrder = 4
      end
      object CreateBackupCheck: TCheckBox
        Left = 10
        Top = 140
        Width = 488
        Height = 21
        Margins.Left = 4
        Margins.Top = 4
        Margins.Right = 4
        Margins.Bottom = 4
        Anchors = [akLeft, akTop, akRight]
        Caption = 'Create backup of database before saving'
        TabOrder = 6
        OnClick = CreateBackupCheckClick
      end
      object MaxNumBackupsBox: TEdit
        Left = 373
        Top = 169
        Width = 57
        Height = 25
        Margins.Left = 4
        Margins.Top = 4
        Margins.Right = 4
        Margins.Bottom = 4
        Anchors = [akTop, akRight]
        NumbersOnly = True
        TabOrder = 8
        Text = '1'
      end
      object MaxNumBackupsSpinBtn: TUpDown
        Left = 430
        Top = 169
        Width = 20
        Height = 25
        Margins.Left = 4
        Margins.Top = 4
        Margins.Right = 4
        Margins.Bottom = 4
        Anchors = [akTop, akRight]
        Associate = MaxNumBackupsBox
        Min = 1
        Max = 999
        Position = 1
        TabOrder = 9
      end
      object OpenDbOnStartupCheck: TCheckBox
        Left = 10
        Top = 233
        Width = 471
        Height = 21
        Margins.Left = 4
        Margins.Top = 4
        Margins.Right = 4
        Margins.Bottom = 4
        Caption = 'Open last used database on startup'
        TabOrder = 11
      end
      object ClearClipCloseLockCheck: TCheckBox
        Left = 10
        Top = 20
        Width = 475
        Height = 21
        Margins.Left = 4
        Margins.Top = 4
        Margins.Right = 4
        Margins.Bottom = 4
        Caption = 'Clear clipboard when closing/locking database'
        TabOrder = 0
      end
      object OpenWindowOnStartupCheck: TCheckBox
        Left = 10
        Top = 204
        Width = 471
        Height = 21
        Margins.Left = 4
        Margins.Top = 4
        Margins.Right = 4
        Margins.Bottom = 4
        Caption = 'Open window on startup'
        TabOrder = 10
      end
      object LockAutoSaveCheck: TCheckBox
        Left = 174
        Top = 111
        Width = 311
        Height = 22
        Margins.Left = 4
        Margins.Top = 4
        Margins.Right = 4
        Margins.Bottom = 4
        Caption = 'Save automatically'
        TabOrder = 5
      end
      object DefaultAutotypeSeqBox: TEdit
        Left = 220
        Top = 386
        Width = 282
        Height = 25
        Margins.Left = 4
        Margins.Top = 4
        Margins.Right = 4
        Margins.Bottom = 4
        Anchors = [akLeft, akTop, akRight]
        TabOrder = 15
      end
      object NumberBackupsCheck: TCheckBox
        Left = 71
        Top = 171
        Width = 295
        Height = 22
        Margins.Left = 4
        Margins.Top = 4
        Margins.Right = 4
        Margins.Bottom = 4
        Anchors = [akLeft, akTop, akRight]
        Caption = 'Number backups consecutively - up to:'
        TabOrder = 7
        OnClick = NumberBackupsCheckClick
      end
      object AutoSaveCheck: TCheckBox
        Left = 10
        Top = 353
        Width = 203
        Height = 26
        Margins.Left = 4
        Margins.Top = 4
        Margins.Right = 4
        Margins.Bottom = 4
        Caption = 'Save automatically:'
        TabOrder = 13
        OnClick = AutoSaveCheckClick
      end
      object AutoSaveList: TComboBox
        Left = 220
        Top = 353
        Width = 282
        Height = 25
        Margins.Left = 4
        Margins.Top = 4
        Margins.Right = 4
        Margins.Bottom = 4
        Style = csDropDownList
        Anchors = [akLeft, akTop, akRight]
        ItemIndex = 0
        TabOrder = 14
        Text = 'After adding/modifying an entry'
        Items.Strings = (
          'After adding/modifying an entry'
          'After every change')
      end
      object WarnExpiredEntriesCheck: TCheckBox
        Left = 10
        Top = 263
        Width = 471
        Height = 21
        Margins.Left = 4
        Margins.Top = 4
        Margins.Right = 4
        Margins.Bottom = 4
        Caption = 'Warn if database contains expired entries'
        TabOrder = 12
      end
      object WarnEntriesExpireSoonCheck: TCheckBox
        Left = 10
        Top = 291
        Width = 471
        Height = 22
        Margins.Left = 4
        Margins.Top = 4
        Margins.Right = 4
        Margins.Bottom = 4
        Caption = 'Warn if database contains entries that will expire soon'
        TabOrder = 16
      end
      object WarnExpireNumDaysBox: TEdit
        Left = 220
        Top = 319
        Width = 61
        Height = 25
        Margins.Left = 4
        Margins.Top = 4
        Margins.Right = 4
        Margins.Bottom = 4
        TabOrder = 17
        Text = '1'
      end
      object WarnExpireNumDaysSpinBtn: TUpDown
        Left = 281
        Top = 319
        Width = 20
        Height = 25
        Margins.Left = 4
        Margins.Top = 4
        Margins.Right = 4
        Margins.Bottom = 4
        Associate = WarnExpireNumDaysBox
        Min = 1
        Position = 1
        TabOrder = 18
      end
    end
  end
  object OKBtn: TButton
    Tag = 12
    Left = 343
    Top = 475
    Width = 93
    Height = 31
    Margins.Left = 4
    Margins.Top = 4
    Margins.Right = 4
    Margins.Bottom = 4
    Caption = 'OK'
    Default = True
    TabOrder = 1
    OnClick = OKBtnClick
  end
  object CancelBtn: TButton
    Tag = 12
    Left = 444
    Top = 475
    Width = 94
    Height = 31
    Margins.Left = 4
    Margins.Top = 4
    Margins.Right = 4
    Margins.Bottom = 4
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
    Left = 69
    Top = 451
  end
  object SaveDlg: TSaveDialog
    Filter = 'Gettext PO files (*.po)|*.po'
    Left = 19
    Top = 452
  end
  object SelectFontMenu: TPopupMenu
    Left = 29
    Top = 470
    object SelectFontMenu_RestoreDefault: TMenuItem
      Caption = 'Restore Default'
      OnClick = SelectFontMenu_RestoreDefaultClick
    end
  end
  object AppIconImageList: TImageList
    Left = 489
    Top = 405
  end
  object OpenDlg: TOpenDialog
    Filter = 'Language files|*.po|All files|*.*'
    Title = 'Select language file to install'
    Left = 31
    Top = 163
  end
end
