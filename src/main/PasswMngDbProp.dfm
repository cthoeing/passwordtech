object PasswMngDbPropDlg: TPasswMngDbPropDlg
  Left = 0
  Top = 0
  BorderIcons = [biSystemMenu, biMaximize]
  Caption = 'Properties'
  ClientHeight = 394
  ClientWidth = 513
  Color = clBtnFace
  Font.Charset = DEFAULT_CHARSET
  Font.Color = clWindowText
  Font.Height = -14
  Font.Name = 'Tahoma'
  Font.Style = []
  OnClose = FormClose
  OnShow = FormShow
  PixelsPerInch = 120
  DesignSize = (
    513
    394)
  TextHeight = 17
  object PropView: TListView
    Left = 0
    Top = 0
    Width = 513
    Height = 341
    Margins.Left = 4
    Margins.Top = 4
    Margins.Right = 4
    Margins.Bottom = 4
    Align = alTop
    Anchors = [akLeft, akTop, akRight, akBottom]
    Columns = <
      item
        Caption = 'Property'
        Width = 188
      end
      item
        Caption = 'Value'
        Width = 300
      end>
    Groups = <
      item
        Header = 'File'
        GroupID = 0
        State = [lgsNormal, lgsSelected]
        HeaderAlign = taLeftJustify
        FooterAlign = taLeftJustify
        TitleImage = -1
      end
      item
        Header = 'Database'
        GroupID = 1
        State = [lgsNormal]
        HeaderAlign = taLeftJustify
        FooterAlign = taLeftJustify
        TitleImage = -1
      end>
    Items.ItemData = {
      05460200000B00000000000000FFFFFFFFFFFFFFFF0000000000000000000000
      00044E0061006D00650000000000FFFFFFFFFFFFFFFF00000000000000000000
      0000084C006F0063006100740069006F006E0000000000FFFFFFFFFFFFFFFF00
      000000000000000000000004530069007A00650000000000FFFFFFFFFFFFFFFF
      0000000000000000000000000D4300720065006100740069006F006E00200074
      0069006D00650000000000FFFFFFFFFFFFFFFF00000000000000000000000011
      4C0061007300740020006D006F00640069006600690063006100740069006F00
      6E0000000000FFFFFFFFFFFFFFFF0000000001000000000000000B4400650073
      006300720069007000740069006F006E0000000000FFFFFFFFFFFFFFFF000000
      0001000000000000000E46006F0072006D006100740020007600650072007300
      69006F006E0000000000FFFFFFFFFFFFFFFF0000000001000000000000001552
      00650063006F0076006500720079002000700061007300730077006F00720064
      00200073006500740000000000FFFFFFFFFFFFFFFF0000000001000000000000
      00114E0075006D0062006500720020006F006600200065006E00740072006900
      6500730000000000FFFFFFFFFFFFFFFF000000000100000000000000194E0075
      006D0062006500720020006F0066002000650078007000690072006500640020
      0065006E007400720069006500730000000000FFFFFFFFFFFFFFFF0000000001
      000000000000000E4E0075006D0062006500720020006F006600200074006100
      67007300}
    GroupView = True
    ReadOnly = True
    RowSelect = True
    TabOrder = 0
    ViewStyle = vsReport
    ExplicitHeight = 328
  end
  object CloseBtn: TButton
    Left = 210
    Top = 353
    Width = 94
    Height = 31
    Margins.Left = 4
    Margins.Top = 4
    Margins.Right = 4
    Margins.Bottom = 4
    Anchors = [akLeft, akBottom]
    Caption = 'Close'
    ModalResult = 1
    TabOrder = 1
    ExplicitTop = 340
  end
end
