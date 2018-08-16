///////////////////////////////////////////////////////////////////////
// AddChildren.xaml.cs - Popup Window to select children             //
// ver 1.0                                                           //
// Vikrant Bhopatrao, CSE687 - Object Oriented Design, Spring 2018   //
///////////////////////////////////////////////////////////////////////
/*
 * Package Operations:
 * -------------------
 * This package provides a window containing a directory structure which lets 
 * you select children for checkin file.
 *   
 * Required Files:
 * ---------------
 * AddChildren.xaml.cs, AddChildren.xaml
 * 
 * Maintenance History:
 * --------------------
 * ver 1.0 : 20 April 2018
 */

using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Data;
using System.Windows.Documents;
using System.Windows.Input;
using System.Windows.Media;
using System.Windows.Media.Imaging;
using System.Windows.Shapes;

namespace ClientGUI
{
    /// <summary>
    /// Interaction logic for AddChildren.xaml
    /// </summary>
    public partial class AddChildren : Window
    {
        private List<string> childs { get; set; } = new List<string>();
        private Stack<RepoStorage> pathStack_ = new Stack<RepoStorage>();
        RepoStorage RS_ = new RepoStorage("");

        //----< constructor to initialize window >-------
        public AddChildren(RepoStorage RS)
        {

            InitializeComponent();
            RS_ = RS;

        }

        //----< Clear directory list >-------
        private void clearDirs()
        {
            DirList.Items.Clear();
        }
        //----< add directory to directory list >-------

        private void addDir(string dir)
        {
            DirList.Items.Add(dir);
        }
        //----< insert parent in directory list >-------

        private void insertParent()
        {
            DirList.Items.Insert(0, "..");
        }
        //----< clear child list  >-------

        private void clearFiles()
        {
            ChildList.Items.Clear();
        }
        //----< add files to child list >-------

        private void addFile(string file)
        {
            ChildList.Items.Add(file);
        }

        //----< remove first directory from filespec>-------
        private string removeFirstDir(string fileName)
        {
            string modifiedName = fileName;
            int pos = fileName.IndexOf("::");
            modifiedName = fileName.Substring(pos + 2, fileName.Length - pos - 2);
            return modifiedName;
        }

        //----< get directory from repostorage >-------
        private void getDir(RepoStorage RS, string dir)
        {
            if (RS.Dir_ == dir)
            {
                RS_ = RS;
                return;
            }
            foreach (RepoStorage child in RS.subDirs_)
            {
                getDir(child, dir);
            }
        }

        //----< initial directory components when window is loaded >-------
        private void Window_Loaded(object sender, RoutedEventArgs e)
        {
            pathStack_.Push(RS_);
            foreach (var dir in RS_.subDirs_)
            {
                addDir(dir.Dir_);
            }
            insertParent();
            foreach (var file in RS_.files_)
            {
                addFile(file);
            }
        }

        //----< add child to the child list >-------
        private void addChild(string key)
        {
            key = getFullFileSpec(key);
            if (childs.Contains(key))
                return;
            childs.Add(key);
            WrapPanel wp = new WrapPanel();
            wp.DataContext = key;
            wp.Margin = new Thickness(5, 5, 5, 5);
            wp.VerticalAlignment = VerticalAlignment.Center;
            wp.Background = Brushes.White;
            TextBlock printTextBlock1 = new TextBlock();
            printTextBlock1.Text = key;
            printTextBlock1.Height = 15;
            wp.Children.Add(printTextBlock1);
            Button btn1 = new Button();
            btn1.DataContext = key;
            btn1.Height = 15;
            btn1.Width = 15;
            btn1.Margin = new Thickness(2, 2, 2, 2);
            btn1.BorderThickness = new Thickness(0);
            btn1.Content = "X";
            wp.Children.Add(btn1);
            children.Children.Add(wp);
            btn1.AddHandler(Button.ClickEvent, new RoutedEventHandler(remove_Click));
        }

        //----< get full filespec from folders>-------
        private string getFullFileSpec(string key)
        {
            foreach (var dir in pathStack_)
            {
                key = dir.Dir_ + "::" + key;
            }
            key = removeFirstDir(key);
            return key;
        }

        //----< remove a child from list >-------
        private void remove_Click(object sender, RoutedEventArgs e)
        {
            string btn = (sender as Button).DataContext.ToString();
            children.Children.Remove(children.Children.OfType<FrameworkElement>().FirstOrDefault(a => a.DataContext.ToString() == btn));
            childs.Remove(btn);
        }

        //----< show files is selected directory >-------
        private void DirList_MouseDoubleClick(object sender, MouseButtonEventArgs e)
        {
            string selectedDir = (string)DirList.SelectedItem;
            if (DirList.SelectedIndex != -1)
            {
                if (selectedDir == "..")
                {
                    if (pathStack_.Count > 1)
                    { 
                        pathStack_.Pop();
                        setPath();
                        RS_ = pathStack_.Peek();
                    }
                    else
                        return;
                }
                else
                {
                    pathStack_.Push(RS_.subDirs_[DirList.SelectedIndex - 1]);
                    RS_ = RS_.subDirs_[DirList.SelectedIndex - 1];
                    setPath();
                }
                clearDirs();
                foreach (var dir in RS_.subDirs_)
                {
                    addDir(dir.Dir_);
                }
                insertParent();
                clearFiles();
                foreach (var file in RS_.files_)
                {
                    addFile(file);
                }
            }
        }

        //----< set path of the current folder >-------
        private void setPath()
        {
            PathTextBlock.Text = "";
            foreach (var dir in pathStack_)
            {
                PathTextBlock.Text = dir.Dir_ + "/" + PathTextBlock.Text;
            }
        }

        //----< add the selected file to child list >-------
        private void FileList_MouseDoubleClick(object sender, MouseButtonEventArgs e)
        {
            if (ChildList.SelectedIndex != -1)
                addChild(ChildList.SelectedItem.ToString());
        }

        //----<close the current window>-------
        private void addChildren_Button_Click(object sender, RoutedEventArgs e)
        {
            this.Close();
        }

        //----< overloaded showdialog() function >-------
        public void ShowDialog(out List<string> s)
        {
            this.ShowDialog();
            s = this.childs;
            return;
        }

        //----< testcase 1 >-------
        public async void automateTest1()
        {
            await PutTaskDelay();
            ChildList.SelectedIndex = 0;
            FileList_MouseDoubleClick(this, null);
            await PutTaskDelay();
            DirList.SelectedIndex = 1;
            DirList_MouseDoubleClick(this, null);
            await PutTaskDelay();
            ChildList.SelectedIndex = 0;
            FileList_MouseDoubleClick(this, null);
            await PutTaskDelay();
            addChildren_Button_Click(this, null);
        }

        //----< testcase 2 >-------
        public async void automateTest2()
        {
            await PutTaskDelay();
            ChildList.SelectedIndex = 0;
            FileList_MouseDoubleClick(this, null);
            await PutTaskDelay();
            DirList.SelectedIndex = 2;
            DirList_MouseDoubleClick(this, null);
            await PutTaskDelay();
            ChildList.SelectedIndex = 0;
            FileList_MouseDoubleClick(this, null);
            await PutTaskDelay();
            addChildren_Button_Click(this, null);
        }

        //----< testcase 3 >-------
        public async void automateTest3()
        {
            await PutTaskDelay();
            ChildList.SelectedIndex = 0;
            FileList_MouseDoubleClick(this, null);
            await PutTaskDelay();
            DirList.SelectedIndex = 2;
            DirList_MouseDoubleClick(this, null);
            await PutTaskDelay();
            ChildList.SelectedIndex = 1;
            FileList_MouseDoubleClick(this, null);
            await PutTaskDelay();
            addChildren_Button_Click(this, null);
        }

        //----< Delay to insert between different tests >-------
        private async Task PutTaskDelay()
        {

            await Task.Delay(1500);
        }
    }
}
