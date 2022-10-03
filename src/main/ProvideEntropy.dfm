object ProvideEntropyDlg: TProvideEntropyDlg
  Left = 247
  Top = 250
  BorderIcons = [biSystemMenu, biMaximize]
  Caption = 'Provide Additional Entropy'
  ClientHeight = 257
  ClientWidth = 369
  Color = clBtnFace
  Font.Charset = ANSI_CHARSET
  Font.Color = clWindowText
  Font.Height = -11
  Font.Name = 'Tahoma'
  Font.Style = []
  OldCreateOrder = False
  OnActivate = FormActivate
  OnShow = FormShow
  DesignSize = (
    369
    257)
  PixelsPerInch = 96
  TextHeight = 13
  object InfoLbl: TLabel
    Left = 8
    Top = 8
    Width = 345
    Height = 33
    Anchors = [akLeft, akTop, akRight]
    AutoSize = False
    Caption = 
      'Enter or paste some text into the box below, press mouse buttons' +
      ', or move your mouse within the window.'
    WordWrap = True
  end
  object CancelBtn: TButton
    Left = 288
    Top = 224
    Width = 75
    Height = 25
    Anchors = [akRight, akBottom]
    Cancel = True
    Caption = 'Cancel'
    ModalResult = 2
    TabOrder = 2
  end
  object OKBtn: TButton
    Left = 208
    Top = 224
    Width = 75
    Height = 25
    Anchors = [akRight, akBottom]
    Caption = 'OK'
    Default = True
    Enabled = False
    TabOrder = 1
    OnClick = OKBtnClick
  end
  object TextBox: TRichEdit
    Left = 8
    Top = 40
    Width = 353
    Height = 177
    Anchors = [akLeft, akTop, akRight, akBottom]
    HideSelection = False
    MaxLength = 2147483645
    PlainText = True
    ScrollBars = ssVertical
    TabOrder = 0
    Zoom = 100
    OnChange = TextBoxChange
    OnStartDrag = TextBoxStartDrag
  end
end
