object PasswMngColDlg: TPasswMngColDlg
  Left = 0
  Top = 0
  BorderIcons = [biSystemMenu, biMaximize]
  Caption = 'Select columns'
  ClientHeight = 232
  ClientWidth = 254
  Color = clBtnFace
  Constraints.MinHeight = 200
  Constraints.MinWidth = 188
  Font.Charset = DEFAULT_CHARSET
  Font.Color = clWindowText
  Font.Height = -11
  Font.Name = 'Tahoma'
  Font.Style = []
  OldCreateOrder = False
  OnShow = FormShow
  DesignSize = (
    254
    232)
  PixelsPerInch = 96
  TextHeight = 13
  object OptionsList: TCheckListBox
    Left = 0
    Top = 0
    Width = 254
    Height = 194
    Align = alTop
    Anchors = [akLeft, akTop, akRight, akBottom]
    Flat = False
    ItemHeight = 18
    Style = lbOwnerDrawFixed
    TabOrder = 0
  end
  object OKBtn: TButton
    Left = 90
    Top = 200
    Width = 75
    Height = 25
    Anchors = [akRight, akBottom]
    Caption = 'OK'
    Default = True
    TabOrder = 1
    OnClick = OKBtnClick
  end
  object CancelBtn: TButton
    Left = 171
    Top = 200
    Width = 75
    Height = 25
    Anchors = [akRight, akBottom]
    Cancel = True
    Caption = 'Cancel'
    ModalResult = 2
    TabOrder = 2
  end
end
