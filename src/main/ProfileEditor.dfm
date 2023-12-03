object ProfileEditDlg: TProfileEditDlg
  Left = 219
  Top = 131
  BorderIcons = [biSystemMenu, biMaximize]
  Caption = 'Profile Editor'
  ClientHeight = 295
  ClientWidth = 381
  Color = clBtnFace
  Font.Charset = DEFAULT_CHARSET
  Font.Color = clWindowText
  Font.Height = -14
  Font.Name = 'Tahoma'
  Font.Style = []
  OnActivate = FormActivate
  OnShow = FormShow
  PixelsPerInch = 120
  TextHeight = 17
  object ProfileNameLbl: TLabel
    Tag = 9
    Left = 13
    Top = 154
    Width = 79
    Height = 17
    Margins.Left = 4
    Margins.Top = 4
    Margins.Right = 4
    Margins.Bottom = 4
    Caption = 'Profile name:'
  end
  object MoveUpBtn: TSpeedButton
    Tag = 6
    Left = 275
    Top = 9
    Width = 29
    Height = 28
    Hint = 'Move up'
    Margins.Left = 4
    Margins.Top = 4
    Margins.Right = 4
    Margins.Bottom = 4
    ImageIndex = 7
    ImageName = 'up'
    Images = MainForm.ImageList16
    Enabled = False
    ParentShowHint = False
    ShowHint = True
    OnClick = MoveUpBtnClick
  end
  object MoveDownBtn: TSpeedButton
    Tag = 6
    Left = 304
    Top = 9
    Width = 29
    Height = 28
    Hint = 'Move down'
    Margins.Left = 4
    Margins.Top = 4
    Margins.Right = 4
    Margins.Bottom = 4
    ImageIndex = 8
    ImageName = 'down'
    Images = MainForm.ImageList16
    Enabled = False
    ParentShowHint = False
    ShowHint = True
    OnClick = MoveDownBtnClick
  end
  object ProfileList: TListBox
    Tag = 15
    Left = 13
    Top = 10
    Width = 254
    Height = 131
    Margins.Left = 4
    Margins.Top = 4
    Margins.Right = 4
    Margins.Bottom = 4
    ItemHeight = 17
    MultiSelect = True
    TabOrder = 0
    OnClick = ProfileListClick
    OnDblClick = ProfileListDblClick
  end
  object SaveAdvancedOptionsCheck: TCheckBox
    Tag = 13
    Left = 13
    Top = 210
    Width = 350
    Height = 21
    Margins.Left = 4
    Margins.Top = 4
    Margins.Right = 4
    Margins.Bottom = 4
    Caption = 'Save "Advanced password options" for this profile'
    TabOrder = 5
  end
  object ConfirmCheck: TCheckBox
    Tag = 6
    Left = 275
    Top = 119
    Width = 92
    Height = 21
    Margins.Left = 4
    Margins.Top = 4
    Margins.Right = 4
    Margins.Bottom = 4
    Caption = 'Confirm'
    TabOrder = 7
  end
  object LoadBtn: TButton
    Tag = 6
    Left = 275
    Top = 41
    Width = 94
    Height = 32
    Margins.Left = 4
    Margins.Top = 4
    Margins.Right = 4
    Margins.Bottom = 4
    Caption = 'Load'
    Enabled = False
    TabOrder = 1
    OnClick = LoadBtnClick
  end
  object AddBtn: TButton
    Tag = 12
    Left = 275
    Top = 172
    Width = 94
    Height = 31
    Margins.Left = 4
    Margins.Top = 4
    Margins.Right = 4
    Margins.Bottom = 4
    Caption = 'Add'
    Enabled = False
    TabOrder = 4
    OnClick = AddBtnClick
  end
  object CloseBtn: TButton
    Tag = 9
    Left = 157
    Top = 250
    Width = 94
    Height = 31
    Margins.Left = 4
    Margins.Top = 4
    Margins.Right = 4
    Margins.Bottom = 4
    Cancel = True
    Caption = 'Close'
    ModalResult = 1
    TabOrder = 6
  end
  object ProfileNameBox: TEdit
    Tag = 13
    Left = 13
    Top = 174
    Width = 254
    Height = 25
    Margins.Left = 4
    Margins.Top = 4
    Margins.Right = 4
    Margins.Bottom = 4
    TabOrder = 3
    OnChange = ProfileNameBoxChange
    OnKeyPress = ProfileNameBoxKeyPress
  end
  object DeleteBtn: TButton
    Tag = 6
    Left = 275
    Top = 80
    Width = 94
    Height = 31
    Margins.Left = 4
    Margins.Top = 4
    Margins.Right = 4
    Margins.Bottom = 4
    Caption = 'Delete'
    Enabled = False
    TabOrder = 2
    OnClick = DeleteBtnClick
  end
end
