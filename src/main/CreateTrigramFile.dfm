object CreateTrigramFileDlg: TCreateTrigramFileDlg
  Left = 213
  Top = 138
  BorderIcons = [biSystemMenu]
  Caption = 'Create Trigram File'
  ClientHeight = 182
  ClientWidth = 386
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
  object SourceFileLbl: TLabel
    Left = 13
    Top = 10
    Width = 230
    Height = 17
    Margins.Left = 4
    Margins.Top = 4
    Margins.Right = 4
    Margins.Bottom = 4
    Caption = 'Source file (e.g., dictionary, word list):'
  end
  object DestFileLbl: TLabel
    Left = 13
    Top = 70
    Width = 169
    Height = 17
    Margins.Left = 4
    Margins.Top = 4
    Margins.Right = 4
    Margins.Bottom = 4
    Caption = 'Destination file (trigram file):'
  end
  object SourceFileBox: TEdit
    Tag = 7
    Left = 13
    Top = 30
    Width = 311
    Height = 25
    Margins.Left = 4
    Margins.Top = 4
    Margins.Right = 4
    Margins.Bottom = 4
    TabOrder = 0
    OnChange = SourceFileBoxChange
  end
  object DestFileBox: TEdit
    Tag = 7
    Left = 13
    Top = 90
    Width = 311
    Height = 25
    Margins.Left = 4
    Margins.Top = 4
    Margins.Right = 4
    Margins.Bottom = 4
    TabOrder = 1
    OnChange = SourceFileBoxChange
  end
  object BrowseBtn: TButton
    Tag = 6
    Left = 332
    Top = 27
    Width = 41
    Height = 31
    Hint = 'Browse'
    Margins.Left = 4
    Margins.Top = 4
    Margins.Right = 4
    Margins.Bottom = 4
    Caption = '...'
    ParentShowHint = False
    ShowHint = True
    TabOrder = 2
    OnClick = BrowseBtnClick
  end
  object BrowseBtn2: TButton
    Tag = 6
    Left = 332
    Top = 87
    Width = 41
    Height = 31
    Hint = 'Browse'
    Margins.Left = 4
    Margins.Top = 4
    Margins.Right = 4
    Margins.Bottom = 4
    Caption = '...'
    ParentShowHint = False
    ShowHint = True
    TabOrder = 3
    OnClick = BrowseBtn2Click
  end
  object CreateFileBtn: TButton
    Tag = 6
    Left = 140
    Top = 140
    Width = 131
    Height = 31
    Margins.Left = 4
    Margins.Top = 4
    Margins.Right = 4
    Margins.Bottom = 4
    Caption = 'Create file'
    Default = True
    Enabled = False
    TabOrder = 4
    OnClick = CreateFileBtnClick
  end
  object CloseBtn: TButton
    Tag = 6
    Left = 279
    Top = 140
    Width = 94
    Height = 31
    Margins.Left = 4
    Margins.Top = 4
    Margins.Right = 4
    Margins.Bottom = 4
    Cancel = True
    Caption = 'Close'
    ModalResult = 2
    TabOrder = 5
  end
end
