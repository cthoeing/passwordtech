object ProfileEditDlg: TProfileEditDlg
  Left = 219
  Top = 131
  BorderIcons = [biSystemMenu, biMaximize]
  Caption = 'Profile Editor'
  ClientHeight = 239
  ClientWidth = 321
  Color = clBtnFace
  Font.Charset = DEFAULT_CHARSET
  Font.Color = clWindowText
  Font.Height = -11
  Font.Name = 'Tahoma'
  Font.Style = []
  OldCreateOrder = False
  OnActivate = FormActivate
  OnShow = FormShow
  DesignSize = (
    321
    239)
  PixelsPerInch = 96
  TextHeight = 13
  object ProfileNameLbl: TLabel
    Left = 8
    Top = 128
    Width = 63
    Height = 13
    Anchors = [akLeft, akBottom]
    Caption = 'Profile name:'
  end
  object MoveUpBtn: TSpeedButton
    Left = 239
    Top = 9
    Width = 23
    Height = 22
    Hint = 'Move up'
    Anchors = [akTop, akRight]
    ImageIndex = 7
    ImageName = 'up'
    Images = MainForm.ImageList16
    Enabled = False
    ParentShowHint = False
    ShowHint = True
    OnClick = MoveUpBtnClick
  end
  object MoveDownBtn: TSpeedButton
    Left = 264
    Top = 8
    Width = 23
    Height = 22
    Hint = 'Move down'
    Anchors = [akTop, akRight]
    ImageIndex = 8
    ImageName = 'down'
    Images = MainForm.ImageList16
    Enabled = False
    ParentShowHint = False
    ShowHint = True
    OnClick = MoveDownBtnClick
  end
  object ProfileList: TListBox
    Left = 8
    Top = 8
    Width = 225
    Height = 110
    Anchors = [akLeft, akTop, akRight, akBottom]
    ItemHeight = 13
    MultiSelect = True
    TabOrder = 0
    OnClick = ProfileListClick
    OnDblClick = ProfileListDblClick
  end
  object SaveAdvancedOptionsCheck: TCheckBox
    Left = 8
    Top = 172
    Width = 305
    Height = 17
    Anchors = [akLeft, akRight, akBottom]
    Caption = 'Save "Advanced password options" for this profile'
    TabOrder = 5
  end
  object ConfirmCheck: TCheckBox
    Left = 239
    Top = 100
    Width = 73
    Height = 17
    Anchors = [akRight, akBottom]
    Caption = 'Confirm'
    TabOrder = 7
  end
  object LoadBtn: TButton
    Left = 239
    Top = 37
    Width = 75
    Height = 25
    Anchors = [akTop, akRight]
    Caption = 'Load'
    Enabled = False
    TabOrder = 1
    OnClick = LoadBtnClick
  end
  object DeleteBtn: TButton
    Left = 239
    Top = 68
    Width = 75
    Height = 25
    Anchors = [akRight, akBottom]
    Caption = 'Delete'
    Enabled = False
    TabOrder = 2
    OnClick = DeleteBtnClick
  end
  object AddBtn: TButton
    Left = 240
    Top = 142
    Width = 75
    Height = 25
    Anchors = [akRight, akBottom]
    Caption = 'Add'
    Enabled = False
    TabOrder = 4
    OnClick = AddBtnClick
  end
  object CloseBtn: TButton
    Left = 128
    Top = 204
    Width = 75
    Height = 25
    Anchors = [akLeft, akBottom]
    Cancel = True
    Caption = 'Close'
    ModalResult = 1
    TabOrder = 6
  end
  object ProfileNameBox: TEdit
    Left = 8
    Top = 144
    Width = 225
    Height = 21
    Anchors = [akLeft, akRight, akBottom]
    TabOrder = 3
    OnChange = ProfileNameBoxChange
    OnKeyPress = ProfileNameBoxKeyPress
  end
end
