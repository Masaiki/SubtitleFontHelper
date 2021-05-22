using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Data;
using System.Windows.Documents;
using System.Windows.Forms;
using System.Windows.Input;
using System.Windows.Media;
using System.Windows.Media.Imaging;
using System.Windows.Shapes;

namespace SubtitleFontHelper
{
    /// <summary>
    /// Interaction logic for IndexBuilder.xaml
    /// </summary>
    public partial class IndexBuilder : Window
    {
        public IndexBuilder()
        {
            InitializeComponent();
        }

        private void AddDirectoryButton_OnClick(object sender, RoutedEventArgs e)
        {
            FolderBrowserDialog folderBrowser = new FolderBrowserDialog();

            folderBrowser.RootFolder = Environment.SpecialFolder.MyComputer;

            var result = folderBrowser.ShowDialog();
            if (result == System.Windows.Forms.DialogResult.OK)
            {
                ListBoxItem item = new ListBoxItem();
                item.Content = folderBrowser.SelectedPath;
                if (DirectoryList.SelectedIndex != -1)
                {
                    DirectoryList.Items.Insert(DirectoryList.SelectedIndex, item);
                }
                else
                {
                    DirectoryList.Items.Add(item);
                }
                
            }
        }

        private void RemoveDirectoryButton_OnClick(object sender, RoutedEventArgs e)
        {
            int idx = DirectoryList.SelectedIndex;
            if (idx == -1) return;
            DirectoryList.Items.RemoveAt(idx);
            if (idx < DirectoryList.Items.Count)
            {
                DirectoryList.SelectedIndex = idx;
            }
            else
            {
                DirectoryList.SelectedIndex = DirectoryList.Items.Count - 1;
            }

        }

        private void MoveUpButton_OnClick(object sender, RoutedEventArgs e)
        {
            int idx = DirectoryList.SelectedIndex;
            if (idx == 0) return;
            object item = DirectoryList.Items[idx];
            DirectoryList.Items.RemoveAt(idx);
            DirectoryList.Items.Insert(idx - 1, item);
            DirectoryList.SelectedIndex = idx - 1;
        }

        private void MoveDownButton_OnClick(object sender, RoutedEventArgs e)
        {
            int idx = DirectoryList.SelectedIndex;
            if (idx == DirectoryList.Items.Count - 1) return;
            object item = DirectoryList.Items[idx];
            DirectoryList.Items.RemoveAt(idx);
            DirectoryList.Items.Insert(idx + 1, item);
            DirectoryList.SelectedIndex = idx + 1;
        }

        private void CancelButton_OnClick(object sender, RoutedEventArgs e)
        {
            throw new NotImplementedException();
        }

        private void NextButton_OnClick(object sender, RoutedEventArgs e)
        {
            throw new NotImplementedException();
        }

        private void PrevButton_OnClick(object sender, RoutedEventArgs e)
        {
            throw new NotImplementedException();
        }

    }
}
