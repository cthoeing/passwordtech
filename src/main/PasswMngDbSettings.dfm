object PasswDbSettingsDlg: TPasswDbSettingsDlg
  Left = 0
  Top = 0
  BorderIcons = [biSystemMenu, biMaximize]
  Caption = 'Database settings'
  ClientHeight = 289
  ClientWidth = 393
  Color = clBtnFace
  Constraints.MinHeight = 279
  Constraints.MinWidth = 377
  Font.Charset = DEFAULT_CHARSET
  Font.Color = clWindowText
  Font.Height = -11
  Font.Name = 'Tahoma'
  Font.Style = []
  OldCreateOrder = False
  OnActivate = FormActivate
  OnClose = FormClose
  OnShow = FormShow
  DesignSize = (
    393
    289)
  PixelsPerInch = 96
  TextHeight = 13
  object ConfigPages: TPageControl
    Left = 8
    Top = 8
    Width = 377
    Height = 240
    ActivePage = SecuritySheet
    Anchors = [akLeft, akTop, akRight, akBottom]
    TabOrder = 0
    object GeneralSheet: TTabSheet
      Caption = 'General'
      DesignSize = (
        369
        212)
      object DefUserNameLbl: TLabel
        Left = 8
        Top = 16
        Width = 168
        Height = 13
        Caption = 'Default user name for new entries:'
      end
      object PasswFormatSeqLbl: TLabel
        Left = 8
        Top = 72
        Width = 291
        Height = 13
        Caption = 'Generate passwords for new entries using format sequence:'
      end
      object PasswGenTestBtn: TSpeedButton
        Left = 334
        Top = 118
        Width = 23
        Height = 22
        Hint = 'Generate test password using format sequence'
        Anchors = [akTop, akRight]
        Flat = True
        Glyph.Data = {
          36030000424D3603000000000000360000002800000010000000100000000100
          1800000000000003000000000000000000000000000000000000FFFFFFFFFFFF
          FFFFFFFFFFFFD9B886BB8333AA680CA25D01A25D01AA680CBB8333D9B886FFFF
          FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFECDCC3BC8434A15C01A96300B06A00C3
          7C00C37C00B37111B06A00A15C01BB8434ECDCC3FFFFFFFFFFFFFFFFFFEDDCC3
          B47821A15C00BD7704C57F06C58006BC7704BC7704C57F06C58006BC7704AA64
          00B57821EDDCC3FFFFFFFFFFFFBD8534A35E00BE7A0AC8850EB87A20A45E00A3
          5E00A35E00A35E00B37217C9850EC9850EA35E00BD8534FFFFFFD9B886A86301
          C17F12CC8B18A66100B06E10CB9E5CEDDDC5EDDDC5CB9E5CB06E10AE6800CC8B
          18CC8B17A86301D9B886BE8633B5720DD09222BD7E20B26F10E2C9A3FFFFFFFF
          FFFFFFFFFFFFFFFFE3C9A3B27010C48419D19123AA6300BE8633B2710CC78922
          D5992EAD6700CCA05CFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFCCA05CAD67
          00AD6700AC6700B2700CB16B01D9A03ADAA13BB16B00EEDEC5FFFFFFFFFFFFFF
          FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFB56F01DEA847
          DEA847B46E00EEDEC5FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF
          FFFFFFFFFFFFFFFFFFFFB9750CD1993CE3B052B87200CEA25CFFFFFFFFFFFFFF
          FFFFFFFFFFB97200B97100B97200B97200B97200B97100B97200C38C33BD8024
          E7B65DC98A20BC7A10E4CAA3FFFFFFFFFFFFFFFFFFE2C188BB7400D59E44E7B6
          5DE7B75EE7B75DBA7300DBBA86BD7701D8A34BEABD67CC8C20BE7B10D0A25CEE
          DEC5EEDEC5CFA25CBE7B10BC7500D8A34AEABC67EABC67BC7500FFFFFFC58E34
          C07900D9A651EDC16FCE8E20C07A00C07900C17A00C07900CD8E20DAA751EDC2
          6FD9A651EEC26FBD7700FFFFFFEDDDC3C38521C27B00DBA955F0C575F0C575DB
          A955DBA955F0C575F0C575DBA955CE8F20BF7800DBA955BF7800FFFFFFFFFFFF
          EDDDC3C68E34C17B01CE8F20D79E37F0C575F0C575D79E37CE8F20C17B01C68E
          34E1C28EC07900C07900FFFFFFFFFFFFFFFFFFFFFFFFDBBA86C68E33C07C0CC1
          7A01C17A01C07C0CC68E33DBBA86FFFFFFFFFFFFE2C188C07900}
        ParentShowHint = False
        ShowHint = True
        OnClick = PasswGenTestBtnClick
      end
      object DefaultExpiryLbl: TLabel
        Left = 8
        Top = 160
        Width = 318
        Height = 13
        Caption = 
          'By default, new entries expire after the following number of day' +
          's:'
      end
      object PasswFormatSeqBox: TEdit
        Left = 8
        Top = 91
        Width = 350
        Height = 21
        Anchors = [akLeft, akTop, akRight]
        TabOrder = 1
      end
      object DefUserNameBox: TEdit
        Left = 8
        Top = 35
        Width = 350
        Height = 21
        Anchors = [akLeft, akTop, akRight]
        TabOrder = 0
      end
      object DefaultÉxpiryBox: TEdit
        Left = 8
        Top = 179
        Width = 57
        Height = 21
        TabOrder = 3
        Text = '0'
      end
      object DefaultExpiryUpDown: TUpDown
        Left = 65
        Top = 179
        Width = 16
        Height = 21
        Associate = DefaultÉxpiryBox
        Max = 3650
        TabOrder = 4
      end
      object PasswGenTestBox: TEdit
        Left = 8
        Top = 118
        Width = 322
        Height = 21
        Anchors = [akLeft, akTop, akRight]
        Color = clBtnFace
        ReadOnly = True
        TabOrder = 2
      end
    end
    object SecuritySheet: TTabSheet
      Caption = 'Security'
      ImageIndex = 1
      DesignSize = (
        369
        212)
      object EncryptionAlgoLbl: TLabel
        Left = 8
        Top = 16
        Width = 102
        Height = 13
        Caption = 'Encryption algorithm:'
      end
      object NumKdfRoundsLbl: TLabel
        Left = 8
        Top = 72
        Width = 161
        Height = 13
        Caption = 'Number of key derivation rounds:'
      end
      object CalcRoundsBtn: TSpeedButton
        Left = 335
        Top = 68
        Width = 23
        Height = 23
        Hint = 'Calculate number of rounds for a 1 second delay'
        Anchors = [akTop, akRight]
        Flat = True
        Glyph.Data = {
          36030000424D3603000000000000360000002800000010000000100000000100
          1800000000000003000000000000000000000000000000000000F5DBABEDC274
          E4B45FDEA741D496242D2D9D2D2D9D2D2D9D2D2D9D2D2D9D2D2D9DD39525E0AA
          44E4B561EEC57AF2D296FFFEFCEEC77AE8B046BE852E2D2D9DA197B1FFF2CDFF
          F2CDFFF2CDFFF2CDA197B12D2D9DBA8331E8B24AEFC579FFFEFBFFFFFFFFFFFE
          7170B7C6BDC9FFF3CEFFF3CEFFF5CEFFF2CDFFF2CDFFF4CEFFF4CEFFF4CECAC1
          CA4645A6FFFFFEFFFFFFFFFFFF6262B6E0D6D0FFF8D5BE852EFFF4CEFFF4CEFF
          F4CEFFF4CEFFF4CEFFF4CEFFF4CEFFF4CEEADFD14C4CACFFFFFFFFFFFF3334A1
          FFFCDCFFFCDCFFFCDCBE852EFFFCDCFFFCDCFFFCDCFFFCDCFFFCDCFFFCDCFFFC
          DCFFFADC2D2D9DFFFFFFD5DAF4DBD1CFFFF9E1FFF9E1FFF9E1FFF9E1BE852EFF
          F9E1FFF9E1FFF9E1FFF9E1FFF9E1FFF9E1FFF9E1EADFD17888DA4F5ABFFBFBE9
          FFFFEAFCF4E7FFFFEBFFFFEBFFFFEBBE852EBE852EBE852EBE852EBE852EBE85
          2EFFFFEBFFFFEB354DC7203BC1FFFFF1FFFFF1FDF7EEFDF7EEFEFBF4FDF7EEBE
          852EFDF7EEFDF7EEFDF7EEFDF7EEFDF7EEFDF7EEFFFEF4203BC1203BC1F7FAF9
          FFFFF9FFFDF6FDFAF5FFFDFAFFFDFABE852EFFFDFAFFFFFEFFFFFEFFFFFDFFFC
          F6FFFEF9FFFCF6203BC1203BC1D2D9EEF7FAF9F7FAF9FFFFFDFFFFFFFFFFFFBE
          852EFFFFFFFEFDFCFEFDFCFFFEFDFFFFFDFEFDFCD2D9EE203BC1203BC11988C1
          0798CFE2F1F8F8F9FBF8F9FBF8F9FBBE852EF8F9FBFFFFFFFFFFFFFFFFFFE8F3
          F9099AD0158CC2203BC159B3D60DAFE209CCFE05B4E9A4D6EBFDFEFEFFFEFDBE
          852EFFFFFFFFFFFFFFFFFFDBEDF507ABE004C9FD0CBBED35A1CE47B7DF2EC7F1
          3FD7FB3DD8FD26BEEAB9DEF2FAFAFEFFFFFFFFFFFFFFFFFFF1F7FC2AB8E33DD7
          FD3FD6FB3FD5FA2BADDBB4E5F52BC0EB81E9FE7DE7FC7CE9FE1EB2E4ECEFFCF7
          F7FEFFFFFFE6E7FA1195DD76E7FB87EBFE8BECFE48CDF17FD2EEFCFEFF45C9F1
          4AD1F578E3FA4CD5F502A5E8122BD7151BD61F2BD51822D6008BE43AD1F281E6
          FA63DAF72AC1EEF1FBFEFFFFFFFCFFFF73DFFA49D0F74DDDF9FFFFFFFFFFFFFF
          FFFFFFFFFFFFFFFFFFFFFF76E4FA43CFF768DBF9F0FDFFFFFFFF}
        ParentShowHint = False
        ShowHint = True
        OnClick = CalcRoundsBtnClick
        ExplicitLeft = 303
      end
      object EncryptionAlgoList: TComboBox
        Left = 8
        Top = 35
        Width = 350
        Height = 21
        Style = csDropDownList
        Anchors = [akLeft, akTop, akRight]
        TabOrder = 0
      end
      object NumKdfRoundsBox: TEdit
        Left = 237
        Top = 69
        Width = 94
        Height = 21
        Anchors = [akTop, akRight]
        TabOrder = 1
      end
    end
  end
  object OKBtn: TButton
    Left = 224
    Top = 254
    Width = 75
    Height = 25
    Anchors = [akRight, akBottom]
    Caption = 'OK'
    Default = True
    TabOrder = 1
    OnClick = OKBtnClick
  end
  object CancelBtn: TButton
    Left = 305
    Top = 254
    Width = 75
    Height = 25
    Anchors = [akRight, akBottom]
    Caption = 'Cancel'
    ModalResult = 2
    TabOrder = 2
  end
end
