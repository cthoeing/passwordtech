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
          0A544A504547496D616765BB0A0000FFD8FFE000104A4649460001010100C800
          C80000FFFE00134372656174656420776974682047494D50FFE202B04943435F
          50524F46494C45000101000002A06C636D73043000006D6E7472524742205859
          5A2007E70002000100140039002B616373704D53465400000000000000000000
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
          1414141414141414141414141414141414141414FFC200110800220292030111
          00021101031101FFC40017000101010100000000000000000000000003020604
          FFC4001B01010101010101010100000000000000000200010305090708FFDA00
          0C03010002100310000001E2FC9FFA0930260B82C122982E09048A67348A4130
          24520982E0B82E2904C09048A413024130242E298120904C2904826048261485
          C2A13822130984DCE30442613098467398442704C26E710984C2304DCE611098
          46733B945C8F391DCE3391DCA2E279CCEE51723B91E728B91DC8EE53832BECFD
          104C0982E0B048A60B82412298120904C0914C09048260B82453024122904C09
          04C0904C3704824130A4120981209852170A84E0884C26117398460984DCE631
          9CE61109C1309845CE61308C1309B9C426119CCEE51723CE47738B89E728B91D
          CCF391DCA2E479CA2E47723B94E0CAFB3F44130242F02C122982E09048A67348
          A413024530242E0B86E0904C09048A413024130242E298120904C09148241305
          E1585C2B04C221309845CE611826137398C67398442704DCE63173984C2304C2
          6E7109846733B945C8F391DCE2E279CE2E27733CE51723B91E728B91DC8EE538
          32BECFD104C0982E0B0B8A60482412299CD22904C0914C090B82614824130241
          22904C0904C090B8A604824130A4120981209852170A84E0884C261173984609
          84C26119CE61109C1309B9C4261308C1309B9C427045CCEE51723CE47738B89E
          728B91DCCF3945C8EE479CA2E47723B94E0CAFB3F441302604D299CEE2981209
          048A604824130246F02C2E0B82E0914C09048A60485C170242E0B86E0904C091
          4826048246F0242A138221309845CE70442613098467398442704DCE63173984
          42704C26E710984603B8C5C8F391DCE2E279CA2E67723CE51723B91E728B91DC
          8EE5247FFFC4001A100100030101010000000000000000000001001020305040
          FFDA0008010100010502210A3890B210D10A28B21E53B30713051B21643064F0
          9A789660C1DC868F40E2590A3068A3D17651464A210851828A2182182193EE6D
          B72D7FFFC400191101010101010100000000000000000000010010203050FFDA
          0008010301013F01D7E13CBAEB38CF2EBD3333333333333333F25D759E9E9D79
          6666666666666667E7B8CCE33CBACF2CCCCCCCCCCCCCCCFCF71F2671E9999999
          999999999ED9EDC7C9F175F0671D6719D75D71999999999999999CFFC4001411
          0100000000000000000000000000000080FFDA0008010201013F014FFF00FFC4
          0014100100000000000000000000000000000080FFDA0008010100063F024FFF
          00FFC40017100101010100000000000000000000000001001020FFDA00080101
          00013F21D0608888888D08E011111A1830468444444CCCCCCCCCCCCCCCCCCCCC
          CCCCCCCCCCCCCE19999D11182222222304460C111111A11A111822230CCCCCCC
          CCCCCCCCCCCCCCCCCCCCCCCCCE1C3333384608C1118223044444444444446844
          4460888899999999999999999999999999999999999999999D11111111823430
          44608888C182222222222222666666666666666666666666666666666670CCCC
          E8C18222307206088C18342234223422230E3333333333333333333333333338
          670CE1999D7FFFDA000C0301000200030000001019F697F3C2C055C9E808B73E
          B84652B5E48AB20E6E4FB09BE5D3E2CD6431B684FB4BF9A16122FC76005B5F5C
          23297AF25658473725D8C9F2E170F63618D9C23DB5BCB4B0975A7B022DCFED10
          9CA7592A3C23A99AC8E4E870E873190C6DA33EFEDE72584AED3D8016E7F708CA
          56BC959601DDC9F67275BA7C3D8C863670D8736B312C65DF91C4BB757BC4E47D
          5E4AAF20FEE6BE9930453F3E86431B7FFFC40018110101010101000000000000
          00000000000100104020FFDA0008010301013F106670CCCCCCCCCCCCCCCCCCCC
          CCCCCCCCCCCCE1999C3386747466670CE19999E2000066666666666666666666
          66666666666666666670CE19D19999C333338670CCCF10000333333333333333
          33333333333333333333333333333387C07466670CE8CCCF1000033333333333
          3333333333333333333333333333333333870CCCCCCCCE8E19999E2020066667
          46666670E199999999C333333338670CCCCCCE8E19D1D19C338670F100107FFF
          C4001E11000104010500000000000000000000000120304041100011215060FF
          DA0008010201013F109B508C22C178C52B3D496799D520CDDFCA9C9C95D2AD25
          15A382C8916E157FFFC4001B1000020301010100000000000000000000103100
          0141202130FFDA0008010100013F10862E39FC9D68105C18B99BE32547050597
          23D451C8AD4D4B7A4D4689C060F1351BA024D0244FB7FEF62B64F5DEEF33C768
          A2F5CAE79F4A2A8250D7DAE6F26D03068109AE06E306B90D7037DFFB2EB9F5B0
          B1CBE4B91C1D631DE8727C8E45E2D47880F5178F8E7F334449BF880689A9AE06
          A3767AD9F677BE7F61072393E47037F624633D458F45C8D5196B975E3BD75FD4
          6268109AE06BE0034443EC56CEB7F0DEB7C181C73F88FD05A2C7A8B12E3D46B1
          C44178B4458A2971A28E63CC4D4B64D828681A24DCDC60D731A8D16245B17B1D
          0BD82D99EC5EE6CBFFD9}
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
