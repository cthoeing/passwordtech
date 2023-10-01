object PasswMngColDlg: TPasswMngColDlg
  Left = 0
  Top = 0
  BorderIcons = [biSystemMenu, biMaximize]
  Caption = 'Select columns'
  ClientHeight = 290
  ClientWidth = 325
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
  DesignSize = (
    325
    290)
  TextHeight = 17
  object OptionsList: TCheckListBox
    Left = 0
    Top = 0
    Width = 325
    Height = 231
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
  end
  object OKBtn: TButton
    Left = 113
    Top = 245
    Width = 93
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
    Left = 214
    Top = 245
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
  end
end
