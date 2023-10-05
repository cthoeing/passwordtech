object PasswDbSettingsDlg: TPasswDbSettingsDlg
  Left = 0
  Top = 0
  BorderIcons = [biSystemMenu, biMaximize]
  Caption = 'Database settings'
  ClientHeight = 434
  ClientWidth = 488
  Color = clBtnFace
  Constraints.MinHeight = 349
  Constraints.MinWidth = 405
  Font.Charset = DEFAULT_CHARSET
  Font.Color = clWindowText
  Font.Height = -14
  Font.Name = 'Tahoma'
  Font.Style = []
  OnActivate = FormActivate
  OnClose = FormClose
  OnShow = FormShow
  PixelsPerInch = 120
  DesignSize = (
    488
    434)
  TextHeight = 17
  object Label1: TLabel
    Left = 130
    Top = 30
    Width = 39
    Height = 17
    Margins.Left = 4
    Margins.Top = 4
    Margins.Right = 4
    Margins.Bottom = 4
    Caption = 'Label1'
  end
  object ConfigPages: TPageControl
    Left = 13
    Top = 10
    Width = 466
    Height = 364
    Margins.Left = 4
    Margins.Top = 4
    Margins.Right = 4
    Margins.Bottom = 4
    ActivePage = GeneralSheet
    Anchors = [akLeft, akTop, akRight, akBottom]
    TabOrder = 0
    object GeneralSheet: TTabSheet
      Margins.Left = 4
      Margins.Top = 4
      Margins.Right = 4
      Margins.Bottom = 4
      Caption = 'General'
      DesignSize = (
        458
        332)
      object DefUserNameLbl: TLabel
        Left = 10
        Top = 20
        Width = 210
        Height = 17
        Margins.Left = 4
        Margins.Top = 4
        Margins.Right = 4
        Margins.Bottom = 4
        Caption = 'Default user name for new entries:'
      end
      object PasswFormatSeqLbl: TLabel
        Left = 10
        Top = 90
        Width = 365
        Height = 17
        Margins.Left = 4
        Margins.Top = 4
        Margins.Right = 4
        Margins.Bottom = 4
        Caption = 'Generate passwords for new entries using format sequence:'
      end
      object PasswGenTestBtn: TSpeedButton
        Left = 415
        Top = 148
        Width = 28
        Height = 27
        Hint = 'Generate test password using format sequence'
        Margins.Left = 4
        Margins.Top = 4
        Margins.Right = 4
        Margins.Bottom = 4
        Anchors = [akTop, akRight]
        ImageIndex = 0
        ImageName = 'reload'
        Images = MainForm.ImageList16
        Flat = True
        ParentShowHint = False
        ShowHint = True
        OnClick = PasswGenTestBtnClick
        ExplicitLeft = 403
      end
      object DefaultExpiryLbl: TLabel
        Left = 10
        Top = 200
        Width = 397
        Height = 17
        Margins.Left = 4
        Margins.Top = 4
        Margins.Right = 4
        Margins.Bottom = 4
        Caption = 
          'By default, new entries expire after the following number of day' +
          's:'
      end
      object PasswHistoryLbl: TLabel
        Left = 10
        Top = 270
        Width = 406
        Height = 17
        Margins.Left = 4
        Margins.Top = 4
        Margins.Right = 4
        Margins.Bottom = 4
        Caption = 
          'For new entries, save the following number of previous passwords' +
          ':'
      end
      object PasswFormatSeqBox: TEdit
        Left = 10
        Top = 114
        Width = 433
        Height = 25
        Margins.Left = 4
        Margins.Top = 4
        Margins.Right = 4
        Margins.Bottom = 4
        Anchors = [akLeft, akTop, akRight]
        TabOrder = 1
        ExplicitWidth = 411
      end
      object DefUserNameBox: TEdit
        Left = 10
        Top = 44
        Width = 433
        Height = 25
        Margins.Left = 4
        Margins.Top = 4
        Margins.Right = 4
        Margins.Bottom = 4
        Anchors = [akLeft, akTop, akRight]
        TabOrder = 0
        ExplicitWidth = 411
      end
      object DefaultÉxpiryBox: TEdit
        Left = 10
        Top = 224
        Width = 71
        Height = 25
        Margins.Left = 4
        Margins.Top = 4
        Margins.Right = 4
        Margins.Bottom = 4
        TabOrder = 3
        Text = '0'
      end
      object DefaultExpirySpinBtn: TUpDown
        Left = 81
        Top = 224
        Width = 20
        Height = 25
        Margins.Left = 4
        Margins.Top = 4
        Margins.Right = 4
        Margins.Bottom = 4
        Associate = DefaultÉxpiryBox
        Max = 3650
        TabOrder = 4
      end
      object PasswGenTestBox: TEdit
        Left = 10
        Top = 148
        Width = 398
        Height = 25
        Margins.Left = 4
        Margins.Top = 4
        Margins.Right = 4
        Margins.Bottom = 4
        Anchors = [akLeft, akTop, akRight]
        Color = clBtnFace
        ReadOnly = True
        TabOrder = 2
        ExplicitWidth = 376
      end
      object PasswHistoryBox: TEdit
        Left = 10
        Top = 294
        Width = 71
        Height = 25
        Margins.Left = 4
        Margins.Top = 4
        Margins.Right = 4
        Margins.Bottom = 4
        TabOrder = 5
        Text = '0'
      end
      object PasswHistorySpinBtn: TUpDown
        Left = 81
        Top = 294
        Width = 20
        Height = 25
        Margins.Left = 4
        Margins.Top = 4
        Margins.Right = 4
        Margins.Bottom = 4
        Associate = PasswHistoryBox
        Max = 3650
        TabOrder = 6
      end
    end
    object CompressionSheet: TTabSheet
      Margins.Left = 4
      Margins.Top = 4
      Margins.Right = 4
      Margins.Bottom = 4
      Caption = 'Compression'
      ImageIndex = 2
      object CompressionLevelLbl: TLabel
        Left = 4
        Top = 103
        Width = 431
        Height = 28
        Margins.Left = 4
        Margins.Top = 4
        Margins.Right = 4
        Margins.Bottom = 4
        Alignment = taCenter
        AutoSize = False
        Caption = '<- Low compression, fast | High compression, slow ->'
      end
      object EnableCompressionCheck: TCheckBox
        Left = 10
        Top = 20
        Width = 431
        Height = 21
        Margins.Left = 4
        Margins.Top = 4
        Margins.Right = 4
        Margins.Bottom = 4
        Caption = 'Enable data compression (Deflate)'
        TabOrder = 0
        OnClick = EnableCompressionCheckClick
      end
      object CompressionLevelBar: TTrackBar
        Left = 4
        Top = 49
        Width = 441
        Height = 46
        Margins.Left = 4
        Margins.Top = 4
        Margins.Right = 4
        Margins.Bottom = 4
        Max = 9
        Min = 1
        Position = 6
        TabOrder = 1
        ThumbLength = 25
      end
    end
    object SecuritySheet: TTabSheet
      Margins.Left = 4
      Margins.Top = 4
      Margins.Right = 4
      Margins.Bottom = 4
      Caption = 'Security'
      ImageIndex = 1
      DesignSize = (
        458
        332)
      object EncryptionAlgoLbl: TLabel
        Left = 10
        Top = 20
        Width = 133
        Height = 17
        Margins.Left = 4
        Margins.Top = 4
        Margins.Right = 4
        Margins.Bottom = 4
        Caption = 'Encryption algorithm:'
      end
      object NumKdfRoundsLbl: TLabel
        Left = 10
        Top = 90
        Width = 207
        Height = 17
        Margins.Left = 4
        Margins.Top = 4
        Margins.Right = 4
        Margins.Bottom = 4
        Caption = 'Number of key derivation rounds:'
      end
      object CalcRoundsBtn: TSpeedButton
        Left = 416
        Top = 85
        Width = 29
        Height = 29
        Hint = 'Calculate number of rounds for a 1 second delay'
        Margins.Left = 4
        Margins.Top = 4
        Margins.Right = 4
        Margins.Bottom = 4
        Anchors = [akTop, akRight]
        ImageIndex = 15
        ImageName = '013-alarm-clock'
        Images = PasswMngForm.ImageList16
        Flat = True
        ParentShowHint = False
        ShowHint = True
        OnClick = CalcRoundsBtnClick
        ExplicitLeft = 404
      end
      object EncryptionAlgoList: TComboBox
        Left = 10
        Top = 44
        Width = 435
        Height = 25
        Margins.Left = 4
        Margins.Top = 4
        Margins.Right = 4
        Margins.Bottom = 4
        Style = csDropDownList
        Anchors = [akLeft, akTop, akRight]
        TabOrder = 0
      end
      object NumKdfRoundsBox: TEdit
        Left = 291
        Top = 86
        Width = 118
        Height = 25
        Margins.Left = 4
        Margins.Top = 4
        Margins.Right = 4
        Margins.Bottom = 4
        Anchors = [akTop, akRight]
        TabOrder = 1
      end
    end
  end
  object OKBtn: TButton
    Left = 283
    Top = 388
    Width = 94
    Height = 31
    Margins.Left = 4
    Margins.Top = 4
    Margins.Right = 4
    Margins.Bottom = 4
    Anchors = [akRight, akBottom]
    Caption = 'OK'
    Default = True
    TabOrder = 1
    OnClick = OKBtnClick
  end
  object CancelBtn: TButton
    Left = 385
    Top = 388
    Width = 94
    Height = 31
    Margins.Left = 4
    Margins.Top = 4
    Margins.Right = 4
    Margins.Bottom = 4
    Anchors = [akRight, akBottom]
    Caption = 'Cancel'
    ModalResult = 2
    TabOrder = 2
  end
end
