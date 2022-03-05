object MPPasswGenForm: TMPPasswGenForm
  Left = 219
  Top = 131
  Caption = 'MP Password Generator'
  ClientHeight = 511
  ClientWidth = 395
  Color = clBtnFace
  Font.Charset = ANSI_CHARSET
  Font.Color = clWindowText
  Font.Height = -11
  Font.Name = 'Tahoma'
  Font.Style = []
  OldCreateOrder = False
  PopupMode = pmExplicit
  OnActivate = FormActivate
  OnClose = FormClose
  OnShow = FormShow
  DesignSize = (
    395
    511)
  PixelsPerInch = 96
  TextHeight = 13
  object MasterPasswGroup: TGroupBox
    Left = 8
    Top = 8
    Width = 377
    Height = 265
    Anchors = [akLeft, akTop, akRight]
    Caption = 'Master password'
    TabOrder = 0
    DesignSize = (
      377
      265)
    object PasswStatusLbl: TLabel
      Left = 8
      Top = 64
      Width = 35
      Height = 13
      Caption = 'Status:'
    end
    object PasswExpiryCountdownLbl: TLabel
      Left = 256
      Top = 82
      Width = 3
      Height = 13
    end
    object KeyExpiryInfoLbl: TLabel
      Left = 272
      Top = 64
      Width = 90
      Height = 13
      Anchors = [akTop, akRight]
      Caption = 'Expiry countdown:'
    end
    object KeyExpiryCountdownLbl: TLabel
      Left = 272
      Top = 84
      Width = 3
      Height = 13
      Anchors = [akTop, akRight]
      Font.Charset = ANSI_CHARSET
      Font.Color = clBlack
      Font.Height = -11
      Font.Name = 'Tahoma'
      Font.Style = []
      ParentFont = False
    end
    object AutotypeLbl: TLabel
      Left = 10
      Top = 236
      Width = 98
      Height = 13
      Caption = 'Autotype sequence:'
    end
    object EnterPasswBtn: TButton
      Left = 40
      Top = 24
      Width = 185
      Height = 25
      Caption = 'Enter password...'
      Font.Charset = ANSI_CHARSET
      Font.Color = clWindowText
      Font.Height = -11
      Font.Name = 'Tahoma'
      Font.Style = [fsBold]
      ParentFont = False
      TabOrder = 0
      OnClick = EnterPasswBtnClick
    end
    object ConfirmPasswCheck: TCheckBox
      Left = 8
      Top = 112
      Width = 361
      Height = 17
      Caption = 'Ask for password confirmation'
      TabOrder = 3
    end
    object ShowPasswHashCheck: TCheckBox
      Left = 8
      Top = 135
      Width = 258
      Height = 17
      Caption = 'Show checksum of password:'
      TabOrder = 4
      OnClick = ShowPasswHashCheckClick
    end
    object KeyExpiryCheck: TCheckBox
      Left = 8
      Top = 158
      Width = 258
      Height = 17
      Caption = 'Key expires after the following time (seconds):'
      TabOrder = 5
      OnClick = KeyExpiryCheckClick
    end
    object KeyExpiryTimeBox: TEdit
      Left = 272
      Top = 155
      Width = 49
      Height = 21
      Anchors = [akTop, akRight]
      TabOrder = 6
      Text = '1'
    end
    object KeyExpiryTimeSpinBtn: TUpDown
      Left = 321
      Top = 155
      Width = 16
      Height = 21
      Anchors = [akTop, akRight]
      Associate = KeyExpiryTimeBox
      Max = 32767
      Position = 1
      TabOrder = 7
    end
    object PasswStatusBox: TEdit
      Left = 9
      Top = 83
      Width = 257
      Height = 21
      Anchors = [akLeft, akTop, akRight]
      Color = clBtnFace
      ReadOnly = True
      TabOrder = 2
    end
    object ClearKeyBtn: TButton
      Left = 272
      Top = 24
      Width = 97
      Height = 25
      Anchors = [akTop, akRight]
      Caption = 'Clear'
      Enabled = False
      TabOrder = 1
      OnClick = ClearKeyBtnClick
    end
    object HashapassCompatCheck: TCheckBox
      Left = 8
      Top = 182
      Width = 361
      Height = 17
      Caption = 'Provide compatibility with "Hashapass"'
      TabOrder = 8
      OnClick = HashapassCompatCheckClick
    end
    object AddPasswLenToParamCheck: TCheckBox
      Left = 8
      Top = 205
      Width = 354
      Height = 17
      Caption = 'Add password length to parameter'
      TabOrder = 9
    end
    object PasswHashList: TComboBox
      Left = 273
      Top = 128
      Width = 89
      Height = 21
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
      Left = 160
      Top = 233
      Width = 202
      Height = 21
      Anchors = [akLeft, akTop, akRight]
      TabOrder = 11
    end
  end
  object PasswGeneratorGroup: TGroupBox
    Left = 8
    Top = 279
    Width = 377
    Height = 191
    Anchors = [akLeft, akRight, akBottom]
    Caption = 'Password generator'
    TabOrder = 1
    DesignSize = (
      377
      191)
    object ParameterLbl: TLabel
      Left = 8
      Top = 24
      Width = 54
      Height = 13
      Caption = 'Parameter:'
      OnMouseMove = ParameterLblMouseMove
    end
    object CharSetLbl: TLabel
      Left = 8
      Top = 72
      Width = 70
      Height = 13
      Caption = 'Character set:'
    end
    object LengthLbl: TLabel
      Left = 272
      Top = 72
      Width = 37
      Height = 13
      Anchors = [akTop, akRight]
      Caption = 'Length:'
    end
    object ResultingPasswLbl: TLabel
      Left = 8
      Top = 120
      Width = 113
      Height = 13
      Caption = 'Resulting password:'
      Font.Charset = ANSI_CHARSET
      Font.Color = clBlack
      Font.Height = -11
      Font.Name = 'Tahoma'
      Font.Style = [fsBold]
      ParentFont = False
      OnMouseMove = PasswSecurityBarMouseMove
    end
    object TogglePasswBtn: TSpeedButton
      Left = 242
      Top = 134
      Width = 27
      Height = 25
      Hint = 'Hide/show password'
      AllowAllUp = True
      Anchors = [akTop, akRight]
      GroupIndex = 1
      Caption = #183#183#183
      Flat = True
      Font.Charset = SYMBOL_CHARSET
      Font.Color = clMaroon
      Font.Height = -15
      Font.Name = 'Symbol'
      Font.Style = []
      ParentFont = False
      ParentShowHint = False
      ShowHint = True
      OnClick = TogglePasswBtnClick
    end
    object PasswInfoLbl: TLabel
      Left = 248
      Top = 166
      Width = 3
      Height = 13
      Font.Charset = ANSI_CHARSET
      Font.Color = clBlack
      Font.Height = -11
      Font.Name = 'Tahoma'
      Font.Style = []
      ParentFont = False
      Visible = False
    end
    object CharSetInfoLbl: TLabel
      Left = 260
      Top = 72
      Width = 3
      Height = 13
      Alignment = taRightJustify
      Anchors = [akTop, akRight]
    end
    object ParameterBox: TEdit
      Left = 8
      Top = 40
      Width = 257
      Height = 21
      Anchors = [akLeft, akTop, akRight]
      TabOrder = 0
      OnKeyPress = ParameterBoxKeyPress
    end
    object CharSetList: TComboBox
      Left = 8
      Top = 88
      Width = 257
      Height = 21
      Style = csDropDownList
      Anchors = [akLeft, akTop, akRight]
      TabOrder = 2
      OnChange = CharSetListChange
    end
    object PasswLengthBox: TEdit
      Left = 272
      Top = 88
      Width = 81
      Height = 21
      Anchors = [akTop, akRight]
      TabOrder = 3
      Text = '1'
    end
    object PasswLengthSpinBtn: TUpDown
      Left = 353
      Top = 88
      Width = 16
      Height = 21
      Anchors = [akTop, akRight]
      Associate = PasswLengthBox
      Min = 1
      Position = 1
      TabOrder = 4
    end
    object ClearParameterBtn: TButton
      Left = 272
      Top = 38
      Width = 97
      Height = 25
      Anchors = [akTop, akRight]
      Caption = 'Clear'
      TabOrder = 1
      OnClick = ClearParameterBtnClick
    end
    object PasswBox: TEdit
      Left = 8
      Top = 136
      Width = 233
      Height = 21
      Anchors = [akLeft, akTop, akRight]
      Font.Charset = ANSI_CHARSET
      Font.Color = clWindowText
      Font.Height = -11
      Font.Name = 'Tahoma'
      Font.Style = []
      ParentFont = False
      PopupMenu = PasswBoxMenu
      TabOrder = 5
    end
    object GenerateBtn: TButton
      Left = 272
      Top = 134
      Width = 97
      Height = 25
      Anchors = [akTop, akRight]
      Caption = 'Generate'
      Default = True
      Enabled = False
      Font.Charset = ANSI_CHARSET
      Font.Color = clWindowText
      Font.Height = -11
      Font.Name = 'Tahoma'
      Font.Style = [fsBold]
      ParentFont = False
      TabOrder = 6
      OnClick = GenerateBtnClick
    end
    object PasswSecurityBarPanel: TPanel
      Left = 8
      Top = 162
      Width = 233
      Height = 25
      BevelOuter = bvNone
      TabOrder = 7
      Visible = False
      object PasswSecurityBar: TImage
        Left = 0
        Top = 3
        Width = 233
        Height = 17
        ParentShowHint = False
        Picture.Data = {
          0A544A504547496D61676524030000FFD8FFE000104A46494600010101004800
          480000FFFE00134372656174656420776974682047494D50FFDB004300030202
          0302020303030304030304050805050404050A070706080C0A0C0C0B0A0B0B0D
          0E12100D0E110E0B0B1016101113141515150C0F171816141812141514FFDB00
          430103040405040509050509140D0B0D14141414141414141414141414141414
          1414141414141414141414141414141414141414141414141414141414141414
          1414FFC00011080011028003012200021101031101FFC4001900010101010101
          00000000000000000000000302010506FFC4001B100101010101010101000000
          00000000000001023141512132FFC4001B010003010101010100000000000000
          00000003040502010809FFC40018110101010101000000000000000000000000
          02010331FFDA000C03010002110311003F00FA1CAB84B2AE1F15EBED8D522B9E
          2538AE785693AA4533D4E2992B49D572A613CF54C15A46A99522795215A4D2B9
          F16C259FC53056934AE56CF89655C93A452B9E2B989655C7213A9E96C755CA38
          EAD92748D56718D373F96345E22E89D4F4A693A6632FA27A62B7A629B8C9EA9E
          93AA693A6632FA2753AA693A6632FAB37D66B57ACDE3BC414CDEB2D565D24A66
          B37C6AB35DA4A67ACB57F59749299ACD9FADD66BAC4B4C566B559AEB12532CD6
          AF59AEF1253359AD566BD476CD72F1DAE3B4B40012D1E078049453D2812D38EF
          21E9E04B47A1E8135007A125007812D3AE1E9E0494EB8E812D0004B40012D000
          49600197D400327A80064F5000C9EA00193D5E6E55C7014EBF78754CF15CF00A
          D2B548A63A04E91AA4EAB9E01744529952740AD2695C78AE00AD2695CF17CF80
          4D114A45B3E013A9E94C756CF009A2354FACE80BC45D52BE31AE0198CBE89EBD
          628198CAEA9E98BD037195D12BDAC5E8198CBEAC33E03BC415E3359F80E92533
          F59BD0769298F6B9780E9253959A0E92D3178C83AC476CB941DA4B62B37B41D6
          24A67C7283A4B44F09DA012D1E14024A72BBE804B44278012D1E97D009A83D00
          928BC2804B47D74024A1CFA012D3A004B40012D0004B6E7D3E8065F52F0FA019
          1D5DF080195D4F000C8E84F5DF80F191D5FFD9}
        ShowHint = True
        Stretch = True
        OnMouseMove = PasswSecurityBarMouseMove
      end
    end
  end
  object UseAsDefaultRNGBtn: TButton
    Left = 8
    Top = 476
    Width = 249
    Height = 25
    Anchors = [akLeft, akBottom]
    Caption = 'Use as default random generator'
    Enabled = False
    TabOrder = 2
    OnClick = UseAsDefaultRNGBtnClick
  end
  object CloseBtn: TButton
    Left = 304
    Top = 476
    Width = 83
    Height = 25
    Anchors = [akRight, akBottom]
    Cancel = True
    Caption = 'Close'
    TabOrder = 3
    OnClick = CloseBtnClick
  end
  object KeyExpiryTimer: TTimer
    Enabled = False
    OnTimer = KeyExpiryTimerTimer
    Left = 328
    Top = 96
  end
  object PasswBoxMenu: TPopupMenu
    OnPopup = PasswBoxMenuPopup
    Left = 216
    Top = 372
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
