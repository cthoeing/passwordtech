object PasswMngColDlg: TPasswMngColDlg
  Left = 0
  Top = 0
  BorderIcons = [biSystemMenu, biMaximize]
  Caption = 'Select Columns'
  ClientHeight = 289
  ClientWidth = 328
  Color = clBtnFace
  Constraints.MinHeight = 250
  Constraints.MinWidth = 235
  Font.Charset = DEFAULT_CHARSET
  Font.Color = clWindowText
  Font.Height = -14
  Font.Name = 'Tahoma'
  Font.Style = []
  OnShow = FormShow
  PixelsPerInch = 120
  TextHeight = 17
  object OptionsList: TCheckListBox
    Left = 0
    Top = 0
    Width = 328
    Height = 230
    Margins.Left = 4
    Margins.Top = 4
    Margins.Right = 4
    Margins.Bottom = 4
    Align = alTop
    Anchors = [akLeft, akTop, akRight, akBottom]
    Flat = False
    ItemHeight = 23
    Style = lbOwnerDrawFixed
    TabOrder = 0
    ExplicitWidth = 313
    ExplicitHeight = 229
  end
  object OKBtn: TButton
    Tag = 12
    Left = 121
    Top = 244
    Width = 93
    Height = 31
    Margins.Left = 4
    Margins.Top = 4
    Margins.Right = 4
    Margins.Bottom = 4
    Caption = 'OK'
    Default = True
    TabOrder = 1
    OnClick = OKBtnClick
  end
  object CancelBtn: TButton
    Tag = 12
    Left = 222
    Top = 244
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
end
