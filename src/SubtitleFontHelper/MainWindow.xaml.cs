using libSubtitleFontHelper;
using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.IO;
using System.Linq;
using System.Text;
using System.Threading;
using System.Threading.Tasks;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Data;
using System.Windows.Documents;
using System.Windows.Input;
using System.Windows.Media;
using System.Windows.Media.Imaging;
using System.Windows.Navigation;
using System.Windows.Shapes;
using System.Xml.Serialization;


namespace SubtitleFontHelper
{
    /// <summary>
    /// MainWindow.xaml 的交互逻辑
    /// </summary>
    public partial class MainWindow : Window
    {
        public MainWindow()
        {
            InitializeComponent();
        }


        private void Button_Click(object sender, RoutedEventArgs e)
        {
            btnScan.IsEnabled = false;

            new Thread(delegate ()
              {
                  FontScanner scanner = new FontScanner();
                  List<string> files = FontScanner.ScanDirectory(
                      @"E:\超级字体整合包 XZ\",
                      new string[] { ".ttf", ".ttc", ".otf", ".otc" }
                      );
                  FontFileCollection collection = null;
                  new Thread(delegate ()
                  {
                      collection = scanner.ScanFont(files);
                  }).Start();

                  do
                  {
                      Tuple<string, int> status_tuple = scanner.GetStatusTuple();
                      Thread.Sleep(50);
                      btnScan.Dispatcher.Invoke(delegate ()
                      {
                          InfoL.Content = status_tuple.Item1;
                          pbarScan.Value = (float)status_tuple.Item2 / files.Count * pbarScan.Maximum;
                      });
                      if (status_tuple.Item2 >= files.Count) break;
                  } while (true);

                  btnScan.Dispatcher.Invoke(delegate ()
                  {
                      btnScan.IsEnabled = true;
                  });
              }).Start();
        }

        private void btnCheck_Click(object sender, RoutedEventArgs e)
        {
            FreeTypeFontMetadata fontMetadata = new FreeTypeFontMetadata();
            fontMetadata.OpenFontFile(@"E:\超级字体整合包 XZ\完整包\其他\日文\RICOH（理光）\HGPrettyFrankHS.ttf");
            IFreeTypeFontFaceMetadata fontFaceMetadata = fontMetadata.GetFontFace(0);
            FontFace face = FontFace.FromMetadata(fontFaceMetadata);
        }
    }
}
