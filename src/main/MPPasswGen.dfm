object MPPasswGenForm: TMPPasswGenForm
  Left = 219
  Top = 131
  Caption = 'MP Password Generator'
  ClientHeight = 633
  ClientWidth = 480
  Color = clBtnFace
  Font.Charset = ANSI_CHARSET
  Font.Color = clWindowText
  Font.Height = -14
  Font.Name = 'Tahoma'
  Font.Style = []
  PopupMode = pmExplicit
  OnActivate = FormActivate
  OnClose = FormClose
  OnShow = FormShow
  PixelsPerInch = 120
  TextHeight = 17
  object MasterPasswGroup: TGroupBox
    Tag = 7
    Left = 13
    Top = 9
    Width = 452
    Height = 331
    Margins.Left = 4
    Margins.Top = 4
    Margins.Right = 4
    Margins.Bottom = 4
    Caption = 'Master password'
    TabOrder = 0
    DesignSize = (
      452
      331)
    object PasswStatusLbl: TLabel
      Left = 10
      Top = 80
      Width = 44
      Height = 17
      Margins.Left = 4
      Margins.Top = 4
      Margins.Right = 4
      Margins.Bottom = 4
      Caption = 'Status:'
    end
    object PasswExpiryCountdownLbl: TLabel
      Left = 320
      Top = 103
      Width = 4
      Height = 17
      Margins.Left = 4
      Margins.Top = 4
      Margins.Right = 4
      Margins.Bottom = 4
    end
    object KeyExpiryInfoLbl: TLabel
      Left = 321
      Top = 80
      Width = 118
      Height = 17
      Margins.Left = 4
      Margins.Top = 4
      Margins.Right = 4
      Margins.Bottom = 4
      Anchors = [akTop, akRight]
      Caption = 'Expiry countdown:'
      ExplicitLeft = 325
    end
    object KeyExpiryCountdownLbl: TLabel
      Left = 321
      Top = 105
      Width = 4
      Height = 17
      Margins.Left = 4
      Margins.Top = 4
      Margins.Right = 4
      Margins.Bottom = 4
      Anchors = [akTop, akRight]
      Font.Charset = ANSI_CHARSET
      Font.Color = clBlack
      Font.Height = -14
      Font.Name = 'Tahoma'
      Font.Style = []
      ParentFont = False
      ExplicitLeft = 325
    end
    object AutotypeLbl: TLabel
      Left = 13
      Top = 295
      Width = 124
      Height = 17
      Margins.Left = 4
      Margins.Top = 4
      Margins.Right = 4
      Margins.Bottom = 4
      Caption = 'Autotype sequence:'
    end
    object EnterPasswBtn: TButton
      Left = 50
      Top = 30
      Width = 231
      Height = 31
      Margins.Left = 4
      Margins.Top = 4
      Margins.Right = 4
      Margins.Bottom = 4
      Caption = 'Enter password...'
      Font.Charset = ANSI_CHARSET
      Font.Color = clWindowText
      Font.Height = -14
      Font.Name = 'Tahoma'
      Font.Style = [fsBold]
      ParentFont = False
      TabOrder = 0
      OnClick = EnterPasswBtnClick
    end
    object ConfirmPasswCheck: TCheckBox
      Left = 10
      Top = 140
      Width = 451
      Height = 21
      Margins.Left = 4
      Margins.Top = 4
      Margins.Right = 4
      Margins.Bottom = 4
      Caption = 'Ask for password confirmation'
      TabOrder = 3
    end
    object ShowPasswHashCheck: TCheckBox
      Left = 10
      Top = 169
      Width = 323
      Height = 21
      Margins.Left = 4
      Margins.Top = 4
      Margins.Right = 4
      Margins.Bottom = 4
      Caption = 'Show checksum of password:'
      TabOrder = 4
      OnClick = ShowPasswHashCheckClick
    end
    object KeyExpiryCheck: TCheckBox
      Left = 10
      Top = 198
      Width = 323
      Height = 21
      Margins.Left = 4
      Margins.Top = 4
      Margins.Right = 4
      Margins.Bottom = 4
      Caption = 'Key expires after the following time (seconds):'
      TabOrder = 5
      OnClick = KeyExpiryCheckClick
    end
    object KeyExpiryTimeBox: TEdit
      Left = 321
      Top = 198
      Width = 61
      Height = 25
      Margins.Left = 4
      Margins.Top = 4
      Margins.Right = 4
      Margins.Bottom = 4
      Anchors = [akTop, akRight]
      TabOrder = 6
      Text = '1'
    end
    object KeyExpiryTimeSpinBtn: TUpDown
      Left = 382
      Top = 198
      Width = 20
      Height = 25
      Margins.Left = 4
      Margins.Top = 4
      Margins.Right = 4
      Margins.Bottom = 4
      Anchors = [akTop, akRight]
      Associate = KeyExpiryTimeBox
      Max = 32767
      Position = 1
      TabOrder = 7
    end
    object PasswStatusBox: TEdit
      Left = 11
      Top = 104
      Width = 303
      Height = 25
      Margins.Left = 4
      Margins.Top = 4
      Margins.Right = 4
      Margins.Bottom = 4
      Anchors = [akLeft, akTop, akRight]
      Color = clBtnFace
      ReadOnly = True
      TabOrder = 2
    end
    object ClearKeyBtn: TButton
      Left = 318
      Top = 30
      Width = 121
      Height = 31
      Margins.Left = 4
      Margins.Top = 4
      Margins.Right = 4
      Margins.Bottom = 4
      Anchors = [akTop, akRight]
      Caption = 'Clear'
      Enabled = False
      TabOrder = 1
      OnClick = ClearKeyBtnClick
    end
    object HashapassCompatCheck: TCheckBox
      Left = 10
      Top = 228
      Width = 451
      Height = 21
      Margins.Left = 4
      Margins.Top = 4
      Margins.Right = 4
      Margins.Bottom = 4
      Caption = 'Provide compatibility with "Hashapass"'
      TabOrder = 8
      OnClick = HashapassCompatCheckClick
    end
    object AddPasswLenToParamCheck: TCheckBox
      Left = 10
      Top = 256
      Width = 443
      Height = 22
      Margins.Left = 4
      Margins.Top = 4
      Margins.Right = 4
      Margins.Bottom = 4
      Caption = 'Add password length to parameter'
      TabOrder = 9
    end
    object PasswHashList: TComboBox
      Left = 322
      Top = 165
      Width = 112
      Height = 25
      Margins.Left = 4
      Margins.Top = 4
      Margins.Right = 4
      Margins.Bottom = 4
      Style = csDropDownList
      Anchors = [akTop, akRight]
      ItemIndex = 2
      TabOrder = 10
      Text = 'Hex 16-bit'
      Items.Strings = (
        'Dec 0-99'
        'Dec 0-9999'
        'Hex 16-bit'
        'Hex 32-bit')
    end
    object AutotypeBox: TEdit
      Left = 200
      Top = 291
      Width = 234
      Height = 25
      Margins.Left = 4
      Margins.Top = 4
      Margins.Right = 4
      Margins.Bottom = 4
      Anchors = [akLeft, akTop, akRight]
      TabOrder = 11
    end
  end
  object PasswGeneratorGroup: TGroupBox
    Tag = 13
    Left = 13
    Top = 344
    Width = 452
    Height = 238
    Margins.Left = 4
    Margins.Top = 4
    Margins.Right = 4
    Margins.Bottom = 4
    Caption = 'Password generator'
    TabOrder = 1
    DesignSize = (
      452
      238)
    object ParameterLbl: TLabel
      Left = 10
      Top = 30
      Width = 68
      Height = 17
      Margins.Left = 4
      Margins.Top = 4
      Margins.Right = 4
      Margins.Bottom = 4
      Caption = 'Parameter:'
      OnMouseMove = ParameterLblMouseMove
    end
    object CharSetLbl: TLabel
      Left = 10
      Top = 90
      Width = 87
      Height = 17
      Margins.Left = 4
      Margins.Top = 4
      Margins.Right = 4
      Margins.Bottom = 4
      Caption = 'Character set:'
    end
    object LengthLbl: TLabel
      Left = 321
      Top = 90
      Width = 48
      Height = 17
      Margins.Left = 4
      Margins.Top = 4
      Margins.Right = 4
      Margins.Bottom = 4
      Anchors = [akTop, akRight]
      Caption = 'Length:'
      ExplicitLeft = 325
    end
    object ResultingPasswLbl: TLabel
      Left = 10
      Top = 150
      Width = 140
      Height = 17
      Margins.Left = 4
      Margins.Top = 4
      Margins.Right = 4
      Margins.Bottom = 4
      Caption = 'Resulting password:'
      Font.Charset = ANSI_CHARSET
      Font.Color = clBlack
      Font.Height = -14
      Font.Name = 'Tahoma'
      Font.Style = [fsBold]
      ParentFont = False
    end
    object TogglePasswBtn: TSpeedButton
      Left = 284
      Top = 168
      Width = 33
      Height = 31
      Hint = 'Hide/show password'
      Margins.Left = 4
      Margins.Top = 4
      Margins.Right = 4
      Margins.Bottom = 4
      AllowAllUp = True
      Anchors = [akTop, akRight]
      GroupIndex = 1
      Caption = #183#183#183
      Flat = True
      Font.Charset = SYMBOL_CHARSET
      Font.Color = clMaroon
      Font.Height = -19
      Font.Name = 'Symbol'
      Font.Style = []
      ParentFont = False
      ParentShowHint = False
      ShowHint = True
      OnClick = TogglePasswBtnClick
    end
    object PasswInfoLbl: TLabel
      Left = 310
      Top = 208
      Width = 4
      Height = 17
      Margins.Left = 4
      Margins.Top = 4
      Margins.Right = 4
      Margins.Bottom = 4
      Font.Charset = ANSI_CHARSET
      Font.Color = clBlack
      Font.Height = -14
      Font.Name = 'Tahoma'
      Font.Style = []
      ParentFont = False
      ParentShowHint = False
      ShowHint = True
      Visible = False
      OnMouseMove = PasswInfoLblMouseMove
    end
    object CharSetInfoLbl: TLabel
      Left = 306
      Top = 90
      Width = 4
      Height = 17
      Margins.Left = 4
      Margins.Top = 4
      Margins.Right = 4
      Margins.Bottom = 4
      Alignment = taRightJustify
      Anchors = [akTop, akRight]
      ExplicitLeft = 310
    end
    object PasswGauge: TCGauge
      Left = 10
      Top = 204
      Width = 292
      Height = 21
      Hint = 'Hold left mouse button to drag & drop password'
      Margins.Left = 4
      Margins.Top = 4
      Margins.Right = 4
      Margins.Bottom = 4
      Color = clBtnFace
      ShowText = False
      BackColor = clBtnFace
      MaxValue = 128
      ParentColor = False
      Visible = False
    end
    object ParameterBox: TEdit
      Left = 10
      Top = 50
      Width = 302
      Height = 25
      Margins.Left = 4
      Margins.Top = 4
      Margins.Right = 4
      Margins.Bottom = 4
      Anchors = [akLeft, akTop, akRight]
      TabOrder = 0
      OnKeyPress = ParameterBoxKeyPress
    end
    object CharSetList: TComboBox
      Left = 10
      Top = 110
      Width = 302
      Height = 25
      Margins.Left = 4
      Margins.Top = 4
      Margins.Right = 4
      Margins.Bottom = 4
      Style = csDropDownList
      Anchors = [akLeft, akTop, akRight]
      TabOrder = 2
      OnChange = CharSetListChange
    end
    object PasswLengthBox: TEdit
      Left = 321
      Top = 110
      Width = 101
      Height = 25
      Margins.Left = 4
      Margins.Top = 4
      Margins.Right = 4
      Margins.Bottom = 4
      Anchors = [akTop, akRight]
      TabOrder = 3
      Text = '1'
    end
    object PasswLengthSpinBtn: TUpDown
      Left = 422
      Top = 110
      Width = 20
      Height = 25
      Margins.Left = 4
      Margins.Top = 4
      Margins.Right = 4
      Margins.Bottom = 4
      Anchors = [akTop, akRight]
      Associate = PasswLengthBox
      Min = 1
      Position = 1
      TabOrder = 4
    end
    object ClearParameterBtn: TButton
      Left = 320
      Top = 45
      Width = 121
      Height = 31
      Margins.Left = 4
      Margins.Top = 4
      Margins.Right = 4
      Margins.Bottom = 4
      Anchors = [akTop, akRight]
      Caption = 'Clear'
      TabOrder = 1
      OnClick = ClearParameterBtnClick
    end
    object PasswBox: TEdit
      Left = 10
      Top = 170
      Width = 272
      Height = 23
      Margins.Left = 4
      Margins.Top = 4
      Margins.Right = 4
      Margins.Bottom = 4
      Anchors = [akLeft, akTop, akRight]
      Font.Charset = ANSI_CHARSET
      Font.Color = clWindowText
      Font.Height = -13
      Font.Name = 'Consolas'
      Font.Style = []
      ParentFont = False
      PopupMenu = PasswBoxMenu
      TabOrder = 5
    end
    object GenerateBtn: TButton
      Left = 321
      Top = 168
      Width = 121
      Height = 31
      Margins.Left = 4
      Margins.Top = 4
      Margins.Right = 4
      Margins.Bottom = 4
      Anchors = [akTop, akRight]
      Caption = 'Generate'
      Default = True
      Enabled = False
      Font.Charset = ANSI_CHARSET
      Font.Color = clWindowText
      Font.Height = -14
      Font.Name = 'Tahoma'
      Font.Style = [fsBold]
      ParentFont = False
      TabOrder = 6
      OnClick = GenerateBtnClick
    end
  end
  object UseAsDefaultRNGBtn: TButton
    Tag = 9
    Left = 13
    Top = 592
    Width = 311
    Height = 31
    Margins.Left = 4
    Margins.Top = 4
    Margins.Right = 4
    Margins.Bottom = 4
    Caption = 'Use as default random generator'
    Enabled = False
    TabOrder = 2
    OnClick = UseAsDefaultRNGBtnClick
  end
  object CloseBtn: TButton
    Tag = 12
    Left = 361
    Top = 592
    Width = 104
    Height = 31
    Margins.Left = 4
    Margins.Top = 4
    Margins.Right = 4
    Margins.Bottom = 4
    Caption = 'Close'
    TabOrder = 3
    OnClick = CloseBtnClick
  end
  object KeyExpiryTimer: TTimer
    Enabled = False
    OnTimer = KeyExpiryTimerTimer
    Left = 259
    Top = 79
  end
  object PasswBoxMenu: TPopupMenu
    OnPopup = PasswBoxMenuPopup
    Left = 25
    Top = 455
    object PasswBoxMenu_Undo: TMenuItem
      Caption = 'Undo'
      ShortCut = 16474
      OnClick = PasswBoxMenu_UndoClick
    end
    object PasswBoxMenu_N1: TMenuItem
      Caption = '-'
    end
    object PasswBoxMenu_Cut: TMenuItem
      Caption = 'Cut'
      ShortCut = 16472
      OnClick = PasswBoxMenu_CutClick
    end
    object PasswBoxMenu_Copy: TMenuItem
      Caption = 'Copy'
      ShortCut = 16451
      OnClick = PasswBoxMenu_CopyClick
    end
    object PasswBoxMenu_EncryptCopy: TMenuItem
      Caption = 'Encrypt && Copy'
      ShortCut = 16453
      OnClick = PasswBoxMenu_EncryptCopyClick
    end
    object PasswBoxMenu_AddToDatabase: TMenuItem
      Caption = 'Add to Database'
      OnClick = PasswBoxMenu_AddToDatabaseClick
    end
    object PasswBoxMenu_Paste: TMenuItem
      Caption = 'Paste'
      ShortCut = 16470
      OnClick = PasswBoxMenu_PasteClick
    end
    object PasswBoxMenu_PerformAutotype: TMenuItem
      Caption = 'Perform Autotype'
      ShortCut = 16468
      OnClick = PasswBoxMenu_PerformAutotypeClick
    end
    object PasswBoxMenu_Delete: TMenuItem
      Caption = 'Delete'
      OnClick = PasswBoxMenu_DeleteClick
    end
    object PasswBoxMenu_N2: TMenuItem
      Caption = '-'
    end
    object PasswBoxMenu_SelectAll: TMenuItem
      Caption = 'Select All'
      ShortCut = 16449
      OnClick = PasswBoxMenu_SelectAllClick
    end
  end
end
