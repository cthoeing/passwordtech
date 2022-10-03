object PasswMngDbPropDlg: TPasswMngDbPropDlg
  Left = 0
  Top = 0
  BorderIcons = [biSystemMenu, biMaximize]
  Caption = 'Properties'
  ClientHeight = 305
  ClientWidth = 404
  Color = clBtnFace
  Font.Charset = DEFAULT_CHARSET
  Font.Color = clWindowText
  Font.Height = -11
  Font.Name = 'Tahoma'
  Font.Style = []
  OldCreateOrder = False
  OnClose = FormClose
  OnShow = FormShow
  DesignSize = (
    404
    305)
  PixelsPerInch = 96
  TextHeight = 13
  object PropView: TListView
    Left = 0
    Top = 0
    Width = 404
    Height = 262
    Align = alTop
    Anchors = [akLeft, akTop, akRight, akBottom]
    Columns = <
      item
        Caption = 'Property'
        Width = 150
      end
      item
        Caption = 'Value'
        Width = 240
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
      05160200000A00000000000000FFFFFFFFFFFFFFFF0000000000000000000000
      00044E0061006D00650000000000FFFFFFFFFFFFFFFF00000000000000000000
      0000084C006F0063006100740069006F006E0000000000FFFFFFFFFFFFFFFF00
      000000000000000000000004530069007A00650000000000FFFFFFFFFFFFFFFF
      0000000000000000000000000D4300720065006100740069006F006E00200074
      0069006D00650000000000FFFFFFFFFFFFFFFF00000000000000000000000011
      4C0061007300740020006D006F00640069006600690063006100740069006F00
      6E0000000000FFFFFFFFFFFFFFFF0000000001000000000000000E46006F0072
      006D00610074002000760065007200730069006F006E0000000000FFFFFFFFFF
      FFFFFF000000000100000000000000155200650063006F007600650072007900
      2000700061007300730077006F0072006400200073006500740000000000FFFF
      FFFFFFFFFFFF000000000100000000000000114E0075006D0062006500720020
      006F006600200065006E007400720069006500730000000000FFFFFFFFFFFFFF
      FF000000000100000000000000194E0075006D0062006500720020006F006600
      20006500780070006900720065006400200065006E0074007200690065007300
      00000000FFFFFFFFFFFFFFFF0000000001000000000000000E4E0075006D0062
      006500720020006F00660020007400610067007300}
    GroupView = True
    ReadOnly = True
    RowSelect = True
    TabOrder = 0
    ViewStyle = vsReport
  end
  object CloseBtn: TButton
    Left = 184
    Top = 272
    Width = 75
    Height = 25
    Anchors = [akLeft, akBottom]
    Caption = 'Close'
    ModalResult = 1
    TabOrder = 1
  end
end
