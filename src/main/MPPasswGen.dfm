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
        Left = 1
        Top = 2
        Width = 233
        Height = 17
        ParentShowHint = False
        Picture.Data = {
          0A544A504547496D616765EB060000FFD8FFE000104A46494600010101004800
          480000FFFE00134372656174656420776974682047494D50FFE202B04943435F
          50524F46494C45000101000002A06C636D73043000006D6E7472524742205859
          5A2007E700010008000B003B0037616373704D53465400000000000000000000
          00000000000000000000000000000000F6D6000100000000D32D6C636D730000
          0000000000000000000000000000000000000000000000000000000000000000
          000000000000000000000000000D646573630000012000000040637072740000
          01600000003677747074000001980000001463686164000001AC0000002C7258
          595A000001D8000000146258595A000001EC000000146758595A000002000000
          0014725452430000021400000020675452430000021400000020625452430000
          0214000000206368726D0000023400000024646D6E640000025800000024646D
          64640000027C000000246D6C756300000000000000010000000C656E55530000
          00240000001C00470049004D00500020006200750069006C0074002D0069006E
          002000730052004700426D6C756300000000000000010000000C656E55530000
          001A0000001C005000750062006C0069006300200044006F006D00610069006E
          000058595A20000000000000F6D6000100000000D32D73663332000000000001
          0C42000005DEFFFFF325000007930000FD90FFFFFBA1FFFFFDA2000003DC0000
          C06E58595A200000000000006FA0000038F50000039058595A20000000000000
          249F00000F840000B6C458595A2000000000000062970000B787000018D97061
          72610000000000030000000266660000F2A700000D59000013D000000A5B6368
          726D00000000000300000000A3D70000547C00004CCD0000999A000026670000
          0F5C6D6C756300000000000000010000000C656E5553000000080000001C0047
          0049004D00506D6C756300000000000000010000000C656E5553000000080000
          001C0073005200470042FFDB0043000302020302020303030304030304050805
          050404050A070706080C0A0C0C0B0A0B0B0D0E12100D0E110E0B0B1016101113
          141515150C0F171816141812141514FFDB00430103040405040509050509140D
          0B0D141414141414141414141414141414141414141414141414141414141414
          1414141414141414141414141414141414141414FFC200110800110149030111
          00021101031101FFC40017000101010100000000000000000000000003040206
          FFC4001C0101010101000203000000000000000000030100040509020708FFDA
          000C0301000210031000000193EA6FD01442A30BC1A315106885461A215105F0
          D10A883462783441A2151868854416C5980584B09403C2504B01C12C2500F096
          119CC1398372CF39427286E59E7286E5385CBF99F61D442A20BC1A3151068854
          61785441A30D10A883462A20BC2A30D1068854416C5980584B09403C2504B01C
          12C2580E096119CC1B98272CF39427286E509CD3EE4385CBF99F61D442A20BC2
          A30D106885461A20BC2A20D18A883462A20BC2A20D1868854416C5982580B094
          13C0504B01C12C0584E096119CC1398372CF39437284E509CD3EE43F897FFFC4
          00161001010100000000000000000000000000010010FFDA0008010100010502
          2222222222222222222319999999999999999999C22222222222222222223199
          99999999999999999C2222222222222222222319999999999999999999CFFFC4
          0017110101010100000000000000000000000000010210FFDA0008010301013F
          01AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAD34D34D34D35CAAAAAAAAAAAAAAAA
          AAAAAAAAAAAAAAAAAD34D34D34D35CAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAD
          34D34D34D35CFFC40014110100000000000000000000000000000060FFDA0008
          010201013F017DFFC40014100100000000000000000000000000000060FFDA00
          08010100063F027DFFC400171000030100000000000000000000000000013140
          50FFDA0008010100013F2193FF00FF00C2CA000FFF00FF00FF000B28003FFF00
          FF00FC298003FF00FFDA000C0301000200030000001069F9510AC627D538DCA3
          57BA158ECFB487E5477B181A94E3738D5EA8703F22F20EAD17F8416D6389C835
          5EA15AF48BFF00FFC40014110100000000000000000000000000000060FFDA00
          08010301013F107C869A4F7E00924927BF802924D2EDBC3FFFC4001A11010002
          0301000000000000000000000001003010205041FFDA0008010201013F10B9E3
          38788C75F2B6E63A7FFFC4001B10000203010101000000000000000000000041
          011031202130FFDA0008010100013F105139B516AB7A8B553C7CFA06BA030F63
          51ACA2F154517A7A8B7AF3AC721AEC06A30C6861EB020B5D1B814CC198AA5771
          5D41983706E2A9798CC8C318354618D518C48C666BB91ADFFFD9}
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
