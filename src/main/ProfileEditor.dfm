object ProfileEditDlg: TProfileEditDlg
  Left = 219
  Top = 131
  BorderIcons = [biSystemMenu, biMaximize]
  Caption = 'Profile Editor'
  ClientHeight = 297
  ClientWidth = 408
  Color = clBtnFace
  Font.Charset = DEFAULT_CHARSET
  Font.Color = clWindowText
  Font.Height = -14
  Font.Name = 'Tahoma'
  Font.Style = []
  OnActivate = FormActivate
  OnShow = FormShow
  PixelsPerInch = 120
  DesignSize = (
    408
    297)
  TextHeight = 17
  object ProfileNameLbl: TLabel
    Left = 13
    Top = 157
    Width = 79
    Height = 17
    Margins.Left = 4
    Margins.Top = 4
    Margins.Right = 4
    Margins.Bottom = 4
    Anchors = [akLeft, akBottom]
    Caption = 'Profile name:'
  end
  object MoveUpBtn: TSpeedButton
    Left = 305
    Top = 9
    Width = 29
    Height = 28
    Hint = 'Move up'
    Margins.Left = 4
    Margins.Top = 4
    Margins.Right = 4
    Margins.Bottom = 4
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
    Left = 334
    Top = 9
    Width = 29
    Height = 28
    Hint = 'Move down'
    Margins.Left = 4
    Margins.Top = 4
    Margins.Right = 4
    Margins.Bottom = 4
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
    Left = 13
    Top = 10
    Width = 284
    Height = 134
    Margins.Left = 4
    Margins.Top = 4
    Margins.Right = 4
    Margins.Bottom = 4
    Anchors = [akLeft, akTop, akRight, akBottom]
    ItemHeight = 17
    MultiSelect = True
    TabOrder = 0
    OnClick = ProfileListClick
    OnDblClick = ProfileListDblClick
  end
  object SaveAdvancedOptionsCheck: TCheckBox
    Left = 13
    Top = 213
    Width = 386
    Height = 21
    Margins.Left = 4
    Margins.Top = 4
    Margins.Right = 4
    Margins.Bottom = 4
    Anchors = [akLeft, akRight, akBottom]
    Caption = 'Save "Advanced password options" for this profile'
    TabOrder = 5
  end
  object ConfirmCheck: TCheckBox
    Left = 305
    Top = 122
    Width = 92
    Height = 21
    Margins.Left = 4
    Margins.Top = 4
    Margins.Right = 4
    Margins.Bottom = 4
    Anchors = [akRight, akBottom]
    Caption = 'Confirm'
    TabOrder = 7
  end
  object LoadBtn: TButton
    Left = 305
    Top = 43
    Width = 94
    Height = 32
    Margins.Left = 4
    Margins.Top = 4
    Margins.Right = 4
    Margins.Bottom = 4
    Anchors = [akTop, akRight]
    Caption = 'Load'
    Enabled = False
    TabOrder = 1
    OnClick = LoadBtnClick
  end
  object AddBtn: TButton
    Left = 305
    Top = 174
    Width = 94
    Height = 31
    Margins.Left = 4
    Margins.Top = 4
    Margins.Right = 4
    Margins.Bottom = 4
    Anchors = [akRight, akBottom]
    Caption = 'Add'
    Enabled = False
    TabOrder = 4
    OnClick = AddBtnClick
  end
  object CloseBtn: TButton
    Left = 157
    Top = 253
    Width = 94
    Height = 31
    Margins.Left = 4
    Margins.Top = 4
    Margins.Right = 4
    Margins.Bottom = 4
    Anchors = [akLeft, akBottom]
    Cancel = True
    Caption = 'Close'
    ModalResult = 1
    TabOrder = 6
    ExplicitTop = 252
  end
  object ProfileNameBox: TEdit
    Left = 13
    Top = 177
    Width = 284
    Height = 25
    Margins.Left = 4
    Margins.Top = 4
    Margins.Right = 4
    Margins.Bottom = 4
    Anchors = [akLeft, akRight, akBottom]
    TabOrder = 3
    OnChange = ProfileNameBoxChange
    OnKeyPress = ProfileNameBoxKeyPress
  end
  object DeleteBtn: TButton
    Left = 305
    Top = 83
    Width = 94
    Height = 31
    Margins.Left = 4
    Margins.Top = 4
    Margins.Right = 4
    Margins.Bottom = 4
    Anchors = [akRight, akBottom]
    Caption = 'Delete'
    Enabled = False
    TabOrder = 2
    OnClick = DeleteBtnClick
  end
end
