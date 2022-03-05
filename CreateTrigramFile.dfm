object CreateTrigramFileDlg: TCreateTrigramFileDlg
  Left = 213
  Top = 138
  BorderIcons = [biSystemMenu]
  Caption = 'Create Trigram File'
  ClientHeight = 147
  ClientWidth = 340
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
    340
    147)
  PixelsPerInch = 96
  TextHeight = 13
  object SourceFileLbl: TLabel
    Left = 8
    Top = 8
    Width = 186
    Height = 13
    Caption = 'Source file (e.g., dictionary, word list):'
  end
  object DestFileLbl: TLabel
    Left = 8
    Top = 56
    Width = 137
    Height = 13
    Caption = 'Destination file (trigram file):'
  end
  object SourceFileBox: TEdit
    Left = 8
    Top = 24
    Width = 281
    Height = 21
    Anchors = [akLeft, akTop, akRight]
    TabOrder = 0
    OnChange = SourceFileBoxChange
  end
  object DestFileBox: TEdit
    Left = 8
    Top = 72
    Width = 281
    Height = 21
    Anchors = [akLeft, akTop, akRight]
    TabOrder = 1
    OnChange = SourceFileBoxChange
  end
  object BrowseBtn: TButton
    Left = 296
    Top = 22
    Width = 33
    Height = 25
    Hint = 'Browse'
    Anchors = [akTop, akRight]
    Caption = '...'
    ParentShowHint = False
    ShowHint = True
    TabOrder = 2
    OnClick = BrowseBtnClick
  end
  object BrowseBtn2: TButton
    Left = 296
    Top = 70
    Width = 33
    Height = 25
    Hint = 'Browse'
    Anchors = [akTop, akRight]
    Caption = '...'
    ParentShowHint = False
    ShowHint = True
    TabOrder = 3
    OnClick = BrowseBtn2Click
  end
  object CreateFileBtn: TButton
    Left = 144
    Top = 112
    Width = 105
    Height = 25
    Anchors = [akTop, akRight]
    Caption = 'Create file'
    Default = True
    Enabled = False
    TabOrder = 4
    OnClick = CreateFileBtnClick
  end
  object CloseBtn: TButton
    Left = 256
    Top = 112
    Width = 75
    Height = 25
    Anchors = [akTop, akRight]
    Cancel = True
    Caption = 'Close'
    ModalResult = 2
    TabOrder = 5
  end
end
