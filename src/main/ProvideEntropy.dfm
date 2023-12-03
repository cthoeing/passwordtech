object ProvideEntropyDlg: TProvideEntropyDlg
  Left = 247
  Top = 250
  BorderIcons = [biSystemMenu, biMaximize]
  Caption = 'Provide Additional Entropy'
  ClientHeight = 319
  ClientWidth = 454
  Color = clBtnFace
  Font.Charset = ANSI_CHARSET
  Font.Color = clWindowText
  Font.Height = -14
  Font.Name = 'Tahoma'
  Font.Style = []
  OnActivate = FormActivate
  OnShow = FormShow
  PixelsPerInch = 120
  TextHeight = 17
  object InfoLbl: TLabel
    Tag = 7
    Left = 13
    Top = 10
    Width = 428
    Height = 41
    Margins.Left = 4
    Margins.Top = 4
    Margins.Right = 4
    Margins.Bottom = 4
    AutoSize = False
    Caption = 
      'Enter or paste some text into the box below, press mouse buttons' +
      ', or move your mouse within the window.'
    WordWrap = True
  end
  object CancelBtn: TButton
    Tag = 12
    Left = 347
    Top = 275
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
  object OKBtn: TButton
    Tag = 12
    Left = 245
    Top = 275
    Width = 94
    Height = 31
    Margins.Left = 4
    Margins.Top = 4
    Margins.Right = 4
    Margins.Bottom = 4
    Caption = 'OK'
    Default = True
    Enabled = False
    TabOrder = 1
    OnClick = OKBtnClick
  end
  object TextBox: TRichEdit
    Tag = 15
    Left = 13
    Top = 50
    Width = 428
    Height = 212
    Margins.Left = 4
    Margins.Top = 4
    Margins.Right = 4
    Margins.Bottom = 4
    HideSelection = False
    MaxLength = 2147483645
    PlainText = True
    ScrollBars = ssVertical
    TabOrder = 0
    OnChange = TextBoxChange
    OnStartDrag = TextBoxStartDrag
  end
end
