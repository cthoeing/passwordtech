object PasswDbSettingsDlg: TPasswDbSettingsDlg
  Left = 0
  Top = 0
  BorderIcons = [biSystemMenu, biMaximize]
  Caption = 'Database settings'
  ClientHeight = 348
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
    348)
  PixelsPerInch = 96
  TextHeight = 13
  object Label1: TLabel
    Left = 104
    Top = 24
    Width = 31
    Height = 13
    Caption = 'Label1'
  end
  object ConfigPages: TPageControl
    Left = 8
    Top = 8
    Width = 377
    Height = 299
    ActivePage = GeneralSheet
    Anchors = [akLeft, akTop, akRight, akBottom]
    TabOrder = 0
    object GeneralSheet: TTabSheet
      Caption = 'General'
      DesignSize = (
        369
        271)
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
        ImageIndex = 0
        ImageName = 'reload'
        Images = MainForm.ImageList16
        Flat = True
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
      object PasswHistoryLbl: TLabel
        Left = 8
        Top = 216
        Width = 323
        Height = 13
        Caption = 
          'For new entries, save the following number of previous passwords' +
          ':'
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
      object DefaultExpirySpinBtn: TUpDown
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
      object PasswHistoryBox: TEdit
        Left = 8
        Top = 235
        Width = 57
        Height = 21
        TabOrder = 5
        Text = '0'
      end
      object PasswHistorySpinBtn: TUpDown
        Left = 65
        Top = 235
        Width = 16
        Height = 21
        Associate = PasswHistoryBox
        Max = 3650
        TabOrder = 6
      end
    end
    object CompressionSheet: TTabSheet
      Caption = 'Compression'
      ImageIndex = 2
      object CompressionLevelLbl: TLabel
        Left = 3
        Top = 82
        Width = 345
        Height = 23
        Alignment = taCenter
        AutoSize = False
        Caption = '<- Low compression, fast | High compression, slow ->'
      end
      object EnableCompressionCheck: TCheckBox
        Left = 8
        Top = 16
        Width = 345
        Height = 17
        Caption = 'Enable data compression (Deflate)'
        TabOrder = 0
        OnClick = EnableCompressionCheckClick
      end
      object CompressionLevelBar: TTrackBar
        Left = 3
        Top = 39
        Width = 353
        Height = 37
        Max = 9
        Min = 1
        Position = 6
        TabOrder = 1
      end
    end
    object SecuritySheet: TTabSheet
      Caption = 'Security'
      ImageIndex = 1
      DesignSize = (
        369
        271)
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
        ImageIndex = 15
        ImageName = '013-alarm-clock'
        Images = PasswMngForm.ImageList16
        Flat = True
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
    Top = 313
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
    Top = 313
    Width = 75
    Height = 25
    Anchors = [akRight, akBottom]
    Caption = 'Cancel'
    ModalResult = 2
    TabOrder = 2
  end
end
