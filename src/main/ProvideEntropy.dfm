object ProvideEntropyDlg: TProvideEntropyDlg
  Left = 247
  Top = 250
  BorderIcons = [biSystemMenu, biMaximize]
  Caption = 'Provide Additional Entropy'
  ClientHeight = 320
  ClientWidth = 463
  Color = clBtnFace
  Font.Charset = ANSI_CHARSET
  Font.Color = clWindowText
  Font.Height = -14
  Font.Name = 'Tahoma'
  Font.Style = []
  OnActivate = FormActivate
  OnShow = FormShow
  PixelsPerInch = 120
  DesignSize = (
    463
    320)
  TextHeight = 17
  object InfoLbl: TLabel
    Left = 13
    Top = 10
    Width = 427
    Height = 41
    Margins.Left = 4
    Margins.Top = 4
    Margins.Right = 4
    Margins.Bottom = 4
    Anchors = [akLeft, akTop, akRight]
    AutoSize = False
    Caption = 
      'Enter or paste some text into the box below, press mouse buttons' +
      ', or move your mouse within the window.'
    WordWrap = True
    ExplicitWidth = 439
  end
  object CancelBtn: TButton
    Left = 346
    Top = 276
    Width = 94
    Height = 31
    Margins.Left = 4
    Margins.Top = 4
    Margins.Right = 4
    Margins.Bottom = 4
    Anchors = [akRight, akBottom]
    Cancel = True
    Caption = 'Cancel'
    ModalResult = 2
    TabOrder = 2
    ExplicitLeft = 358
    ExplicitTop = 277
  end
  object OKBtn: TButton
    Left = 244
    Top = 276
    Width = 94
    Height = 31
    Margins.Left = 4
    Margins.Top = 4
    Margins.Right = 4
    Margins.Bottom = 4
    Anchors = [akRight, akBottom]
    Caption = 'OK'
    Default = True
    Enabled = False
    TabOrder = 1
    OnClick = OKBtnClick
    ExplicitLeft = 256
    ExplicitTop = 277
  end
  object TextBox: TRichEdit
    Left = 13
    Top = 50
    Width = 427
    Height = 214
    Margins.Left = 4
    Margins.Top = 4
    Margins.Right = 4
    Margins.Bottom = 4
    Anchors = [akLeft, akTop, akRight, akBottom]
    HideSelection = False
    MaxLength = 2147483645
    PlainText = True
    ScrollBars = ssVertical
    TabOrder = 0
    OnChange = TextBoxChange
    OnStartDrag = TextBoxStartDrag
  end
end
