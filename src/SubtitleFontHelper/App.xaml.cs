using System;
using System.Collections.Generic;
using System.Configuration;
using System.Data;
using System.Linq;
using System.Threading.Tasks;
using System.Windows;

namespace SubtitleFontHelper
{

    public partial class App : Application
    {

        private void Application_Startup(object sender, StartupEventArgs e)
        {
            Uri lang_uri = new Uri(@"/Lang/zh_CN.xaml", UriKind.Relative);
            ResourceDictionary lang = LoadComponent(lang_uri) as ResourceDictionary;
            ReplaceLanguage(lang);
            MainWindow window = new MainWindow();
            window.Show();
        }

        private void ReplaceLanguage(ResourceDictionary dictionary)
        {
            ResourceDictionary lang = Resources.MergedDictionaries[0];
            lang.MergedDictionaries.Clear();
            lang.MergedDictionaries.Add(dictionary);
        }

    }
}
