///////////////////////////////////////////////////////////////////////
// MainWindow.xaml.cs - ClientGUI for Project4                       //
// ver 1.1                                                           //
// Vikrant Bhopatrao, CSE687 - Object Oriented Design, Spring 2018   //
///////////////////////////////////////////////////////////////////////
/*
 * Package Operations:
 * -------------------
 * This package provides a WPF-based GUI for Project3. It's 
 * responsibilities are to:
 * - Provide various tabs for Connect, checkIn, checkOut, Browse, viewFile and viewMetadata
 * - Provide a display of directory contents of a remote Server.
 * - It provides a subdirectory list and a filelist for the selected directory.
 * - Provides facilties to start client server at a user-defined port.
 * - checkin file by providing its metadata.
 * - checkout file by providing its key from the repository database
 * - view entire file by providing its key
 * - view file metadata by providing its key
 *   
 * Required Files:
 * ---------------
 * Mainwindow.xaml, MainWindow.xaml.cs
 * Translater.dll
 * DisplayText.xaml, DisplayText.xaml.cs
 * 
 * Maintenance History:
 * --------------------
 * ver 1.1 : 30 April 2018
 * - Added functionality to satisfy project 4 requirements
 * ver 1.0 : 9 April 2018
 */

// Translater has to be statically linked with CommLibWrapper
// - loader can't find Translater.dll dependent CommLibWrapper.dll
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
using System.Windows.Navigation;
using System.Windows.Shapes;
using System.Threading;
using MsgPassingCommunication;
using System.Text.RegularExpressions;

namespace ClientGUI
{
    public partial class MainWindow : Window
    {
        public MainWindow()
        {
            InitializeComponent();
        }

        private Stack<string> pathStack_ = new Stack<string>();
        private Translater translater = new Translater();
        private CsEndPoint endPoint_ = new CsEndPoint();
        private Thread rcvThrd = null;
        private Dictionary<string, Action<CsMessage>> dispatcher_
          = new Dictionary<string, Action<CsMessage>>();
        private CsEndPoint serverEndPoint = new CsEndPoint();
        private Microsoft.Win32.OpenFileDialog openFileDialog = new Microsoft.Win32.OpenFileDialog();
        private Microsoft.Win32.SaveFileDialog saveFileDialog = new Microsoft.Win32.SaveFileDialog();
        private string selectedFile = "";
        private string selectedFilePath = "";
        private string savePath = "";
        private string children = "";
        private RepoStorage root = new RepoStorage("");
        private RepoStorage rs = new RepoStorage("");
        private int dirCount = 1;
        List<checkInFileData> CIFD = new List<checkInFileData>();
        List<queryData> QD = new List<queryData>();
        AddChildren ac;

        //----< process incoming messages on child thread >----------------

        private void processMessages()
        {
            ThreadStart thrdProc = () => {
                while (true)
                {
                    CsMessage msg = translater.getMessage();
                    string msgId = msg.value("command");
                    if (msgId == "clientQuit")
                        break;
                    if (dispatcher_.ContainsKey(msgId))
                        dispatcher_[msgId].Invoke(msg);
                }
            };
            rcvThrd = new Thread(thrdProc);
            rcvThrd.IsBackground = true;
            rcvThrd.Start();
        }
        //----< function dispatched by child thread to main thread >-------

        private void clearDirs()
        {
            DirList.Items.Clear();
        }
        //----< function dispatched by child thread to main thread >-------

        private void addDir(string dir)
        {
            DirList.Items.Add(dir);
        }
        //----< function dispatched by child thread to main thread >-------

        private void insertParent()
        {
            DirList.Items.Insert(0, "..");
        }
        //----< function dispatched by child thread to main thread >-------

        private void clearFiles()
        {
            FileList.Items.Clear();
            KeyList.Items.Clear();
        }
        //----< function dispatched by child thread to main thread >-------

        private void addFile(CsMessage msg)
        {
            try
            {
                int count = Int32.Parse(msg.value("filescount"));
                while (count > 0)
                {
                    FileList.Items.Add(msg.value("file" + count.ToString()));
                    KeyList.Items.Add(msg.value("key" + count.ToString()));
                    count--;
                }
            }
            catch (Exception)
            {
                Console.WriteLine("Could not parse string...");

            }
        }

        //----< display file received form repository >---------------

        private void displayFile(string file)
        {
            viewfile_button.IsEnabled = true;
            if (file == "false")
            {
                statusBarText.Text += "\n The specified file was not found in Repository...\n";
                return;
            }
            statusBarText.Text += "\n Received file from Server: Opening File Text...\n";
            try
            {
                string fileText = System.IO.File.ReadAllText("../SaveFiles/" + file);
                DisplayText DT = new DisplayText();             // Open popup to display file text
                DT.popup_text.Text = fileText;
                DT.Title = file;
                DT.Show();

            }
            catch (Exception)
            {
                statusBarText.Text += "\n Could not open File Text...\n";
            }
            try
            {
                foreach (var filename in System.IO.Directory.GetFiles("../SaveFiles"))
                {
                    System.IO.File.Delete(filename);
                }
            }
            catch (Exception)
            {
                Console.WriteLine("Could not delete files...");

            }
            CsMessage msg = new CsMessage();
            msg.add("to", CsEndPoint.toString(serverEndPoint));
            msg.add("from", CsEndPoint.toString(endPoint_));
            msg.add("command", "clearSendDir");
            try
            {
                translater.postMessage(msg);
            }
            catch (Exception)
            {
                Console.WriteLine("Unable to send message...");

            }
        }

        //----< display checkin acknowledgement in status bar >----------------
        private void updateCheckInLog(CsMessage msg)
        {
            try
            {
                foreach (var file in System.IO.Directory.GetFiles("../SendFiles"))
                {
                    System.IO.File.Delete(file);
                }
            }
            catch (Exception)
            {
                Console.WriteLine("Unable to delete files...");

            }
            if (msg.value("success") == "false")
                statusBarText.Text += "\n Could not Check In File: " + msg.value("filename") + " to Repository: The checkin is causing cyclic dependency..." + "\n";
            else
                statusBarText.Text += "\n File: " + msg.value("filename") + " Checked In to Repository with status: " + msg.value("status") + "\n";
            statusScroll.ScrollToBottom();
            populateDirsFiles();
            checkin_button.IsEnabled = true;
        }

        //----< display checkout acknowledgement and copy file to client storage >----------------
        private void updateCheckOutLog(string success)
        {
            checkout_button.IsEnabled = true;
            if (success == "true" && savePath != "")
            {
                try
                {
                    foreach (var filename in System.IO.Directory.GetFiles("../SaveFiles"))
                    {
                        string fulldestpath = savePath + "/" + System.IO.Path.GetFileName(filename);
                        System.IO.File.Copy(filename, fulldestpath, true);          // Copy file to client folder
                        statusBarText.Text += "\n File Checked Out to " + fulldestpath + "\n";
                    }
                }
                catch (Exception)
                {
                    Console.WriteLine("Could not copy file...");
                }
            }
            else
            {
                statusBarText.Text += "\n The specified file was not found in Repository...\n";
            }
            statusScroll.ScrollToBottom();
            CsMessage msg = new CsMessage();
            msg.add("to", CsEndPoint.toString(serverEndPoint));
            msg.add("from", CsEndPoint.toString(endPoint_));
            msg.add("command", "clearSendDir");
            try
            {
                foreach (var file in System.IO.Directory.GetFiles("../SaveFiles"))
                {
                    System.IO.File.Delete(file);
                }
                translater.postMessage(msg);
            }
            catch (Exception)
            {
                Console.WriteLine("Could not delete files...");
            }          
        }

        //----< display metadata received from repository >----------------
        private void showMetadata(CsMessage msg)
        {
            MetadataText.Text = "Metadata:\n";
            if (msg.value("success") == "true")
            {
                MetadataText.Text += "\n Key: " + VMFileKeyBox.Text;
                MetadataText.Text += "\n Name: " + msg.value("name");
                MetadataText.Text += "\n DateTime: " + msg.value("datetime");
                MetadataText.Text += "\n Description: " + msg.value("description");
                MetadataText.Text += "\n Status: " + msg.value("status");
                MetadataText.Text += "\n Category: " + msg.value("category");
                MetadataText.Text += "\n Filepath: " + msg.value("filepath");
                MetadataText.Text += "\n Children: " + msg.value("children");
                statusBarText.Text += "\n Metadata received from Server...\n";
            }
            else
            {
                statusBarText.Text += "\n The specified file was not found in Repository...\n";
            }
            statusScroll.ScrollToBottom();
            viewdata_button.IsEnabled = true;
        }

        //----< populate repo dir structure used by add childs dialog >----------------
        private void addRepoDir(CsMessage msg)
        {
            getParent(root, msg.value("parentDir"));
            var enumer = msg.attributes.GetEnumerator();
            while (enumer.MoveNext())
            {
                string key = enumer.Current.Key;
                if (key.Contains("dir"))
                {
                    dirCount++;
                    RepoStorage subDir = new RepoStorage(enumer.Current.Value);
                    rs.subDirs_.Add(subDir);
                }
                if (key.Contains("file"))
                {
                    rs.files_.Add(enumer.Current.Value);
                }
            }

        }

        //----< open a new window to select childs >----------------
        private void openChildrenDialog()
        {
            dirCount--;
            if (dirCount == 0)
            {
                ac = new AddChildren(root);

                ac.ShowDialog(out List<string> s);
                children_button.IsEnabled = true;
                foreach (var child in s)
                {
                    children += child + " ";
                }
                dirCount = 1;
            }

        }

        //----< add delegates to dictionary >----------------
        private void addClientProc(string key, Action<CsMessage> clientProc)
        {
            dispatcher_[key] = clientProc;
        }

        //----< load getDirs processing into dispatcher dictionary >-------
        private void DispatcherLoadGetDirs()
        {
            Action<CsMessage> getDirs = (CsMessage rcvMsg) =>
            {
                Action clrDirs = () =>
                {
                    clearDirs();
                };
                Dispatcher.Invoke(clrDirs, new Object[] { });
                var enumer = rcvMsg.attributes.GetEnumerator();
                while (enumer.MoveNext())
                {
                    string key = enumer.Current.Key;
                    if (key.Contains("dir"))
                    {
                        Action<string> doDir = (string dir) =>
                        {
                            addDir(dir);
                        };
                        Dispatcher.Invoke(doDir, new Object[] { enumer.Current.Value });
                    }
                }
                Action insertUp = () =>
                {
                    insertParent();
                };
                Dispatcher.Invoke(insertUp, new Object[] { });
            };
            addClientProc("getDirs", getDirs);
        }

        //----< load getFiles processing into dispatcher dictionary >------
        private void DispatcherLoadGetFiles()
        {
            Action<CsMessage> getFiles = (CsMessage rcvMsg) =>
            {
                Action clrFiles = () =>
                {
                    clearFiles();
                };
                Dispatcher.Invoke(clrFiles, new Object[] { });

                Action<CsMessage> doFile = (CsMessage msg) =>
                {
                    addFile(msg);
                };
                Dispatcher.Invoke(doFile, new Object[] { rcvMsg });
            };
            addClientProc("getFiles", getFiles);
        }

        //----< load sentFile processing into dispatcher dictionary >-------
        private void DispatcherLoadSentFile()
        {
            Action<CsMessage> sentFile = (CsMessage rcvMsg) =>
            {
                if (rcvMsg.attributes.ContainsKey("checkedOut"))
                {
                    Action<string> checkOutLog = (string success) =>
                    {
                        updateCheckOutLog(success);
                    };
                    Dispatcher.Invoke(checkOutLog, new Object[] { rcvMsg.value("checkedOut") });
                }
                else
                {
                    Action<string> checkOutLog = (string file) =>
                    {
                        displayFile(file);
                    };
                    Dispatcher.Invoke(checkOutLog, new Object[] { rcvMsg.value("fileName") });
                }

            };
            addClientProc("sentFile", sentFile);
        }

        //----< load checkedIn processing into dispatcher dictionary >-------
        private void DispatcherLoadCheckedInFile()
        {
            Action<CsMessage> checkedIn = (CsMessage rcvMsg) =>
            {
                Action<CsMessage> checkInLog = (CsMessage msg) =>
                {
                    updateCheckInLog(msg);
                };
                Dispatcher.Invoke(checkInLog, new Object[] { rcvMsg });
            };
            addClientProc("checkedIn", checkedIn);
        }

        //----< load putMetadata processing into dispatcher dictionary >-------
        private void DispatcherLoadPutMetadata()
        {
            Action<CsMessage> putMetadata = (CsMessage rcvMsg) =>
            {
                Action<CsMessage> putData = (CsMessage msg) =>
                {
                    showMetadata(msg);
                };
                Dispatcher.Invoke(putData, new Object[] { rcvMsg });
            };
            addClientProc("putMetadata", putMetadata);
        }

        //----< load repostorage processing into dispatcher directory >----------------
        private void DispatcherLoadaddRepoStorage()
        {
            Action<CsMessage> addRepoStorage = (CsMessage rcvMsg) =>
            {


                Action<CsMessage> addStorage = (CsMessage msg) =>
                {
                    addRepoDir(msg);
                };
                Dispatcher.Invoke(addStorage, new Object[] { rcvMsg });
            };
            addClientProc("addRepoStorage", addRepoStorage);
        }

        //----< load child window processing into dispatcher directory >----------------
        private void DispatcherLoadsentStorage()
        {
            Action<CsMessage> sentStorage = (CsMessage rcvMsg) =>
            {


                Action<CsMessage> sentRepoStorage = (CsMessage msg) =>
                {
                    openChildrenDialog();
                };
                Dispatcher.Invoke(sentRepoStorage, new Object[] { rcvMsg });
            };
            addClientProc("sentStorage", sentStorage);
        }

        //----< load all dispatcher processing >---------------------------

        private void loadDispatcher()
        {
            DispatcherLoadGetDirs();
            DispatcherLoadGetFiles();
            DispatcherLoadSentFile();
            DispatcherLoadCheckedInFile();
            DispatcherLoadPutMetadata();
            DispatcherLoadaddRepoStorage();
            DispatcherLoadsentStorage();
        }


        //----< start automated tests once GUI is loaded >-------
        private void Window_Loaded(object sender, RoutedEventArgs e)
        {
            string[] args = Environment.GetCommandLineArgs();
            if (args.Length > 2)
            {
                test5();
                
            }
            else
            {
                test1();
            }
        }

        //----< strip off name of first part of path >---------------------
        private string removeFirstDir(string path)
        {
            string modifiedPath = path;
            int pos = path.IndexOf("/");
            modifiedPath = path.Substring(pos + 1, path.Length - pos - 1);
            return modifiedPath;
        }

        //----< remove file name from filepath >----------------
        private string modifySavePath(string path)
        {
            string modifiedpath = path;

            int pos = path.LastIndexOf('\\');
            modifiedpath = path.Substring(0, pos);
            return modifiedpath;
        }

        //----< separate name and version from filekey >----------------
        private string[] modifyCOfileKey(string key)
        {
            string[] modifiedkey = new string[2];

            int pos = key.LastIndexOf('.');
            modifiedkey[0] = key.Substring(0, pos);
            modifiedkey[1] = key.Substring(pos + 1, key.Length - pos - 1);
            return modifiedkey;
        }

        //----< respond to mouse double-click on dir name >----------------
        private void DirList_MouseDoubleClick(object sender, MouseButtonEventArgs e)
        {
            // build path for selected dir
            string selectedDir = (string)DirList.SelectedItem;
            if (DirList.SelectedIndex != -1)
            {
                string path;
                if (selectedDir == "..")
                {
                    if (pathStack_.Count > 1)  // don't pop off "Storage"
                        pathStack_.Pop();
                    else
                        return;
                }
                else
                {
                    path = pathStack_.Peek() + "/" + selectedDir;
                    pathStack_.Push(path);
                }
                PathTextBlock.Text = removeFirstDir(pathStack_.Peek());
                // build message to get dirs and post it
                CsMessage msg = new CsMessage();
                msg.add("to", CsEndPoint.toString(serverEndPoint));
                msg.add("from", CsEndPoint.toString(endPoint_));
                msg.add("command", "getDirs");
                msg.add("path", pathStack_.Peek());
                msg.add("namespaces", getFullFileSpec());
                try
                {
                    translater.postMessage(msg);
                }
                catch (Exception)
                {
                    Console.WriteLine("\nUnable to send message...");
                    statusBarText.Text += "\n Unable to send message...\n";
                }
                // build message to get files and post it
                msg.remove("command");
                msg.add("command", "getFiles");
                try
                {
                    translater.postMessage(msg);
                }
                catch (Exception)
                {
                    Console.WriteLine("\nUnable to send message...");
                    statusBarText.Text += "\n Unable to send message...\n";
                }
            }
        }

        //----< respond to mouse double-click on file name >----------------
        private void FileList_MouseDoubleClick(object sender, MouseButtonEventArgs e)
        {
            string selectedFile = (string)FileList.SelectedItem;
            if (FileList.SelectedIndex != -1 && KeyList.SelectedIndex != -1)
            {

                CsMessage msg = new CsMessage();
                msg.add("to", CsEndPoint.toString(serverEndPoint));
                msg.add("from", CsEndPoint.toString(endPoint_));
                msg.add("command", "sendFile");
                if (pathStack_.Count() != 0)
                {
                    string filePath;
                    filePath = pathStack_.Peek() + "/" + selectedFile;
                    msg.add("path", filePath);
                }
                else
                    msg.add("filename", KeyList.SelectedItem.ToString());
                try
                {
                    translater.postMessage(msg);                // Request repository to send file
                }
                catch (Exception)
                {
                    Console.WriteLine("\nUnable to send message...");
                    statusBarText.Text += "\n Unable to send message...\n";
                }
                statusBarText.Text += "\n Requesting File From Server...\n";
                statusScroll.ScrollToBottom();
            }
        }

        //----< Automated test cases >---------------------------------

        //----< Test Case 1 >----------------
        private async void test1()
        {
            Console.WriteLine("\n***Demonstrating Requirement #1***\n");
            Console.WriteLine("\n Standard C++ libraries were used to build the repository server and C#(WPF) was used for the client GUI...\n");
            Console.WriteLine("\n***Demonstrating Requirement #2***\n");
            Console.WriteLine("\n Connecting to Server from port: " + PortTextBox.Text + "...\n");
            await PutTaskDelay();
            Connect_Button_Click(this, null);               // Connect tab demonstration
            Console.WriteLine("\n Connected to Server...\n");
            await PutSmallTaskDelay();
            tabControl.SelectedItem = CheckIn;
            await PutSmallTaskDelay();
            selectedFilePath = System.IO.Path.GetFullPath("../../../../ClientStorage1/repocore.h");
            selectedFile = "repocore.h";
            FileNameBox.Text = "Repository::repocore.h";
            DescripBox.Text = "Repository core";
            CategoryBox.Text = "header library";
            StatusBox.SelectedIndex = 0;
            AddChildren_Button_Click(this, null);
            await PutLargeTaskDelay();
            ac.automateTest1();
            await PutLargeTaskDelay();
            await PutLargeTaskDelay();
            await PutLargeTaskDelay();
            Console.WriteLine("\n Checking-In file...\n File: " + selectedFile + "\n FileName: " + FileNameBox.Text + "\n Description: " + DescripBox.Text + "\n Category: " + CategoryBox.Text + "\n Status: " + StatusBox.SelectionBoxItem + "\n Children: " + children + "\n");
            CheckIn_Button_Click(this, null);               // CheckIn tab demonstration
            Console.WriteLine("\n File Checked In to Repository with status: " + StatusBox.SelectionBoxItem + "\n");
            await PutTaskDelay();
            test2();
        }

        //----< Test Case 2 >----------------
        private async void test2()
        {                      
            selectedFilePath = System.IO.Path.GetFullPath("../../../../ClientStorage1/browse.h");
            selectedFile = "browse.h";
            FileNameBox.Text = "Browsing::browse.h";
            DescripBox.Text = "Implements queries";
            CategoryBox.Text = "header library";
            StatusBox.SelectedIndex = 1;
            AddChildren_Button_Click(this, null);
            await PutLargeTaskDelay();
            await PutLargeTaskDelay();
            ac.automateTest2();
            await PutLargeTaskDelay();
            await PutLargeTaskDelay();
            await PutLargeTaskDelay();
            addAnotherFile_Button_Click(this, null);            // multiple checkin demonstration
            await PutSmallTaskDelay();
            selectedFilePath = System.IO.Path.GetFullPath("../../../../ClientStorage1/browse.cpp");
            selectedFile = "browse.cpp";
            FileNameBox.Text = "Browsing::browse.cpp";
            DescripBox.Text = "Browse Testub";
            CategoryBox.Text = "source .cpp";
            StatusBox.SelectedIndex = 0;
            await PutTaskDelay();
            Console.WriteLine("\n Checking-In multiple files browse.h and browse.cpp \n");
            CheckIn_Button_Click(this, null);
            await PutLargeTaskDelay();
            Console.WriteLine("\n All files Checked In to Repository \n");
            selectedFilePath = System.IO.Path.GetFullPath("../../../../ClientStorage1/repocore.h");
            selectedFile = "repocore.h";
            FileNameBox.Text = "Repository::repocore.h";
            DescripBox.Text = "Repository core";
            CategoryBox.Text = "header library";
            StatusBox.SelectedIndex = 1;
            AddChildren_Button_Click(this, null);
            await PutLargeTaskDelay();
            await PutLargeTaskDelay();
            ac.automateTest3();
            await PutLargeTaskDelay();
            await PutLargeTaskDelay();
            await PutLargeTaskDelay();
            Console.WriteLine("\n Checking-In file...\n File: " + selectedFile + "\n FileName: " + FileNameBox.Text + "\n Description: " + DescripBox.Text + "\n Category: " + CategoryBox.Text + "\n Status: " + StatusBox.SelectionBoxItem + "\n Children: " + children + "\n");
            CheckIn_Button_Click(this, null);                   // circular dependency demonstration
            await PutTaskDelay();
            Console.WriteLine("\n Could not Check In File: the chceckin is causing cyclic dependency\n");
            test3();
        }

        //----< Test Case 3 >----------------
        private async void test3()
        {
            tabControl.SelectedItem = CheckOut;
            COFileKeyBox.Text = "Browsing::browse.h.1";
            savePath = "../../../../ClientStorage2";
            Console.WriteLine("\n Checking out file " + COFileKeyBox.Text + "without its dependencies...\n");
            await PutTaskDelay();
            CheckOut_Button_Click(this, null);              // CheckOut tab demonstration
            Console.WriteLine("\n File Checked Out to " + System.IO.Path.GetFullPath(savePath) + "\n");
            await PutTaskDelay();
            COFileKeyBox.Text = "Repository::repocore.h.1";
            savePath = "../../../../ClientStorage2";
            includeDependencies.IsChecked = true;
            Console.WriteLine("\n Checking out file " + COFileKeyBox.Text + "along with its dependencies...\n");
            await PutTaskDelay();
            CheckOut_Button_Click(this, null);              // CheckOut tab demonstration
            Console.WriteLine("\n File Checked Out to " + System.IO.Path.GetFullPath(savePath) + "along with its dependencies...\n");
            await PutTaskDelay();
            tabControl.SelectedItem = Browse;
            await PutSmallTaskDelay();
            filenameQuery_txt.Text = "browse";
            versionQuery_txt.Text = "1";
            await PutTaskDelay();
            Console.WriteLine("\n Querying database with filename containing 'browse' and version '1'...\n");
            searchDatabase_Button_Click(this, null);
            await PutLargeTaskDelay();
            Console.WriteLine("\n Cancelling database search query...\n");
            cancelSearch_Button_Click(this, null);
            await PutSmallTaskDelay();
            filenameQuery_txt.Clear();
            versionQuery_txt.Clear();
            noDependants.IsChecked = true;
            await PutTaskDelay();
            unionQuery_Button_Click(this, null);
            await PutSmallTaskDelay();
            dependencyQuery_txt.Text = "DateTime";
            categoryQuery_txt.Text = "header";
            await PutTaskDelay();
            Console.WriteLine("\n Querying database for files having no dependants or containing dependancy 'Date' and category 'Header'...\n");
            searchDatabase_Button_Click(this, null);
            await PutLargeTaskDelay();
            Console.WriteLine("\n Cancelling database search query...\n");
            cancelSearch_Button_Click(this, null);
            await PutTaskDelay();
            test4();
        }

        //----< Test Case 4 >----------------
        private async void test4()
        {
            Console.WriteLine("\n***Demonstrating Requirement #3***\n");
            tabControl.SelectedItem = ViewFile;
            Console.WriteLine("\n Viewing full file text of a Repository file...");
            Console.WriteLine("\n Requesting file with key NoSqlDb::dbcore.h.1 from Server...\n");
            await PutSmallTaskDelay();
            VFileKeyBox.Text = "NoSqlDb::dbcore.h.1";
            await PutSmallTaskDelay();
            ViewFile_Button_Click(this, null);              // ViewFile tab demonstration
            Console.WriteLine("\n Received file from Server: Opening File Text...\n");
            await PutTaskDelay();
            tabControl.SelectedItem = ViewMeta;
            Console.WriteLine("\n Viewing metadata of a Repository file...");
            Console.WriteLine("\n Requesting metadata of file NoSqlDb::dbcore.h.1 from Server...\n");
            await PutSmallTaskDelay();
            VMFileKeyBox.Text = "NoSqlDb::dbcore.h.1";
            await PutSmallTaskDelay();
            ViewMetadata_Button_Click(this, null);          // ViewMetadata tab demonstration
            Console.WriteLine("\n Received metadata from Server...\n");
            await PutTaskDelay();
            Console.WriteLine(MetadataText.Text);
            Console.WriteLine("\n***Demonstrating Requirement #7***\n");
            Console.WriteLine("\n Automated tests for all the tabs have been demonstrated above satisfying this requirement...\n");
        }

        //----< Test Case 5 >----------------
        private async void test5()
        {
            PortTextBox.Text = "8081";
            Console.WriteLine("\n Connecting to Server from port: " + PortTextBox.Text + "...\n");
            Connect_Button_Click(this, null);               // Multiple client demonstration
            Console.WriteLine("\n Connected to Server...\n");
            await PutSmallTaskDelay();
            tabControl.SelectedItem = CheckIn;
            await PutSmallTaskDelay();
            selectedFilePath = System.IO.Path.GetFullPath("../../../../ClientStorage1/repocore.cpp");
            selectedFile = "repocore.cpp";
            FileNameBox.Text = "Repository::repocore.cpp";
            DescripBox.Text = "Repository testub";
            CategoryBox.Text = "source .cpp";
            StatusBox.SelectedIndex = 1;
            await PutTaskDelay();
            Console.WriteLine("\n Checking-In file...\n File: " + selectedFile + "\n FileName: " + FileNameBox.Text + "\n Description: " + DescripBox.Text + "\n Category: " + CategoryBox.Text + "\n Status: " + StatusBox.SelectionBoxItem + "\n Children: " + children + "\n");
            CheckIn_Button_Click(this, null);               // CheckIn tab demonstration
            Console.WriteLine("\n File Checked In to Repository with status: " + StatusBox.SelectionBoxItem + "\n");
        }

        //----< Small Delay to insert between different tests >---------------------------------
        private async Task PutSmallTaskDelay()
        {
            await Task.Delay(1500);
        }

        //----< Delay to insert between different tests >---------------------------------
        private async Task PutTaskDelay()
        {
            await Task.Delay(2500);
        }

        //----< Larger Delay to insert between different tests >---------------------------------
        private async Task PutLargeTaskDelay()
        {
            await Task.Delay(3000);
        }

        //----< Stop client receiver and sender threads >---------------------------------
        private void Disconnect_Button_Click(object sender, RoutedEventArgs e)
        {
            CsMessage msg = new CsMessage();
            msg.add("to", CsEndPoint.toString(endPoint_));
            msg.add("from", CsEndPoint.toString(endPoint_));
            msg.add("command", "clientQuit");
            try
            {
                translater.postMessage(msg);
                statusBarText.Text += "\n Shutting down Client...\n";
            }
            catch (Exception)
            {
                Console.WriteLine("\nUnable to send message...");
                statusBarText.Text += "\n Unable to send message...\n";
            }
            try
            {
                translater.stop();
            }
            catch (Exception)
            {
                Console.WriteLine("\nUnable to stop receiver...");
                statusBarText.Text += "\n Unable to send message...\n";
            }
            DirList.Items.Clear();                      // reset GUI
            FileList.Items.Clear();
            Disconnect_Button.IsEnabled = true;
            Connect_Button.IsEnabled = false;
            MetadataText.Text = "Metadata:";
            statusBarText.Text = "Status:";
            disableButtons();

        }

        //----< start Comm, fill Browse tab with dirs and files >------
        private void Connect_Button_Click(object sender, RoutedEventArgs e)
        {
            bool connect = portCheck(PortTextBox.Text);
            if (connect)
            {
                // start Comm
                try
                {
                    endPoint_.machineAddress = AddressTextBox.Text;
                    endPoint_.port = Convert.ToInt32(PortTextBox.Text);

                    if (!translater.listen(endPoint_))
                    {
                        statusBarText.Text += "\n Could not connect to Server: Port already in use...\n";
                        statusScroll.ScrollToBottom();
                        return;
                    }
                    statusBarText.Text += "\n Connected to Server...\n";
                    statusScroll.ScrollToBottom();
                }
                catch (Exception)
                {
                    Console.WriteLine("\nUnable to start client...");
                    statusBarText.Text += "\n Unable to start client...\n";
                }
                // start processing messages
                processMessages();
                // load dispatcher
                loadDispatcher();
                serverEndPoint.machineAddress = "localhost";
                serverEndPoint.port = 8080;
                PathTextBlock.Text = "Storage";
                pathStack_.Push("../Storage");

                populateDirsFiles();

                enableButtons();
            }
        }

        //----< get filespec from filepath >----------------
        private string getFullFileSpec()
        {
            string key = pathStack_.Peek().Replace("/", "::");
            key = removeFirstNS(key);
            return key;
        }

        //----< remove first namespace from filespec >----------------
        private string removeFirstNS(string fileName)
        {
            string modifiedName = fileName;
            int pos = fileName.IndexOf("::", 3);
            if (pos == -1)
                modifiedName = "";
            else
                modifiedName = fileName.Substring(pos + 2, fileName.Length - pos - 2);
            return modifiedName;
        }

        //----< update dirs and files list box >------
        private void populateDirsFiles()
        {
            CsMessage msg = new CsMessage();
            msg.add("to", CsEndPoint.toString(serverEndPoint));
            msg.add("from", CsEndPoint.toString(endPoint_));
            msg.add("command", "getDirs");
            try
            {
                msg.add("path", pathStack_.Peek());
            }
            catch (Exception)
            {
                Console.WriteLine("\nUnable to send message...");

            }

            msg.add("namespaces", getFullFileSpec());
            msg.add("connect", "ConnectToServer");
            try
            {
                translater.postMessage(msg);
            }
            catch (Exception)
            {
                Console.WriteLine("\nUnable to send message...");
                statusBarText.Text += "\n Unable to send message...\n";
            }
            msg.remove("command");
            msg.add("command", "getFiles");
            try
            {
                translater.postMessage(msg);
            }
            catch (Exception)
            {
                Console.WriteLine("\nUnable to send message...");
                statusBarText.Text += "\n Unable to send message...\n";
            }
        }

        //----< open browse window to select a file for checkin >------
        private void SelectFile_Button_Click(object sender, RoutedEventArgs e)
        {
            openFileDialog.Multiselect = false;
            if (openFileDialog.ShowDialog() == true)
            {
                selectedFilePath = openFileDialog.FileName;

                selectedFile = openFileDialog.SafeFileName;
            }
        }

        //----< send checkin file and attributes to repository server >------
        private void CheckIn_Button_Click(object sender, RoutedEventArgs e)
        {
            if (selectedFile != "" && FileNameBox.Text != "") { 
                CIFD.Add(new checkInFileData(selectedFile, selectedFilePath, FileNameBox.Text, DescripBox.Text, CategoryBox.Text, StatusBox.SelectionBoxItem.ToString(), children));
            }
            foreach (var CIFile in CIFD) { 
                statusBarText.Text += "\n Checking-In file...\n File: " + CIFile.file_ + "\n FileName: " + CIFile.filename_ + "\n Description: " + CIFile.description_ + "\n Category: " + CIFile.category_ + "\n Status: " + CIFile.status_ + "\n Children: " + CIFile.childs_ + "\n";
                statusScroll.ScrollToBottom();
                try{
                    string fullpath = System.IO.Path.Combine("../SendFiles/" + CIFile.file_);
                    fullpath = System.IO.Path.GetFullPath(fullpath);
                    System.IO.File.Copy(CIFile.filepath_, fullpath, true);      // Copy file to send folder of client
                }
                catch (Exception){
                    Console.WriteLine("\nUnable to copy file...");
                }
                CsMessage msg = new CsMessage();
                msg.add("to", CsEndPoint.toString(serverEndPoint));
                msg.add("from", CsEndPoint.toString(endPoint_));
                msg.add("file", CIFile.file_);
                try{
                    translater.postMessage(msg);                    // Send file to repository
                    checkin_button.IsEnabled = false;
                }
                catch (Exception){
                    Console.WriteLine("\nUnable to send message...");
                }
                msg.remove("file");
                msg.add("command", "checkIn");
                msg.add("filename", CIFile.file_);
                msg.add("filespec", CIFile.filename_);
                msg.add("description", CIFile.description_);
                msg.add("category", CIFile.category_ + " ");
                msg.add("status", CIFile.status_);
                msg.add("children", CIFile.childs_);
                try{
                    translater.postMessage(msg);                    // Send metadata to repository
                }
                catch (Exception){
                    Console.WriteLine("\nUnable to send message...");
                }
            }
            clearCheckInFields();
            CIFD.Clear();
        }

        //----< checkout file to desired client storage >------
        private void CheckOut_Button_Click(object sender, RoutedEventArgs e)
        {
            Regex pattern = new Regex("[.]");
            if (savePath == "")
            {
                statusBarText.Text += "\n Destination path is not valid...\n";
                statusScroll.ScrollToBottom();
                return;
            }

            if (!pattern.IsMatch(COFileKeyBox.Text))
            {
                statusBarText.Text += "\n The specified filekey is not valid...\n";
                statusScroll.ScrollToBottom();
                return;
            }
            statusBarText.Text += "\n Checking out file " + COFileKeyBox.Text + "...\n";
            statusScroll.ScrollToBottom();
            CsMessage msg = new CsMessage();
            msg.add("to", CsEndPoint.toString(serverEndPoint));
            msg.add("from", CsEndPoint.toString(endPoint_));
            msg.add("command", "checkOutFile");
            string[] keyAndVersion = modifyCOfileKey(COFileKeyBox.Text);
            msg.add("filekey", keyAndVersion[0]);
            msg.add("version", keyAndVersion[1]);
            if (includeDependencies.IsChecked == true)
                msg.add("dependants", "true");
            else
                msg.add("dependants", "false");
            try
            {
                translater.postMessage(msg);
                checkout_button.IsEnabled = false;
            }
            catch (Exception)
            {
                Console.WriteLine("\nUnable to send message...");
                statusBarText.Text += "\n Unable to send message...\n";
            }
        }

        //----< request repository to send file to client >------
        private void ViewFile_Button_Click(object sender, RoutedEventArgs e)
        {
            string File = VFileKeyBox.Text;
            CsMessage msg = new CsMessage();
            msg.add("to", CsEndPoint.toString(serverEndPoint));
            msg.add("from", CsEndPoint.toString(endPoint_));
            msg.add("filename", File);
            msg.add("command", "sendFile");
            try
            {
                translater.postMessage(msg);
                viewfile_button.IsEnabled = false;
            }
            catch (Exception)
            {
                Console.WriteLine("\nUnable to send message...");
                statusBarText.Text += "\n Unable to send message...\n";
            }
            statusBarText.Text += "\n Requesting file " + File + " from Server...\n";
            statusScroll.ScrollToBottom();
        }

        //----< request repository to send file metadata >------
        private void ViewMetadata_Button_Click(object sender, RoutedEventArgs e)
        {
            string selectedFile = VMFileKeyBox.Text;
            CsMessage msg = new CsMessage();
            msg.add("to", CsEndPoint.toString(serverEndPoint));
            msg.add("from", CsEndPoint.toString(endPoint_));
            msg.add("filename", selectedFile);
            msg.add("command", "getMetaData");
            try
            {
                translater.postMessage(msg);
                viewdata_button.IsEnabled = false;
            }
            catch (Exception)
            {
                Console.WriteLine("\nUnable to send message...");
                statusBarText.Text += "\n Unable to send message...\n";
            }
            statusBarText.Text += "\n Requesting metadata from Server...\n";
            statusScroll.ScrollToBottom();
        }

        //----< check if the port number is valid>------
        private bool portCheck(string port)
        {
            Regex num = new Regex("^[0-9]+$");
            int portNo;
            bool parsed = Int32.TryParse(port, out portNo);
            if (parsed)
            {
                if (num.IsMatch(port) && Int32.Parse(port) > 0 && portNo < 2147483647)
                {
                    statusBarText.Text += "\n Connecting to Server from port: " + port + "...\n";

                    statusScroll.ScrollToBottom();

                }
                return true;
            }
            else
            {
                statusBarText.Text += "\n Could not connect to Server: Invalid port number...\n";
                statusScroll.ScrollToBottom();
                return false;
            }
        }

        //----< enable all buttons when client connects to server >------
        private void enableButtons()
        {
            Connect_Button.IsEnabled = false;
            Disconnect_Button.IsEnabled = true;
            checkin_button.IsEnabled = true;
            addAnotherFile_button.IsEnabled = true;
            checkout_button.IsEnabled = true;
            savePath_button.IsEnabled = true;
            viewfile_button.IsEnabled = true;
            viewdata_button.IsEnabled = true;
            browse_button.IsEnabled = true;
            children_button.IsEnabled = true;
            DirList.IsEnabled = true;
            FileList.IsEnabled = true;
            KeyList.IsEnabled = true;
            PortTextBox.IsEnabled = false;
            children_button.IsEnabled = true;
            searchDatabase_button.IsEnabled = true;
            unionQuery_button.IsEnabled = true;
            andQuery_button.IsEnabled = true;
        }

        //----< disalbe all buttons when client disconnects from server >------
        private void disableButtons()
        {
            Connect_Button.IsEnabled = true;
            Disconnect_Button.IsEnabled = false;
            checkin_button.IsEnabled = false;
            addAnotherFile_button.IsEnabled = false;
            checkout_button.IsEnabled = false;
            savePath_button.IsEnabled = false;
            viewfile_button.IsEnabled = false;
            viewdata_button.IsEnabled = false;
            browse_button.IsEnabled = false;
            children_button.IsEnabled = false;
            DirList.IsEnabled = false;
            FileList.IsEnabled = false;
            KeyList.IsEnabled = false;
            selectedFile = "";
            selectedFilePath = "";
            savePath = "";
            children = "";
            PortTextBox.IsEnabled = true;
            children_button.IsEnabled = false;
            searchDatabase_button.IsEnabled = false;
            unionQuery_button.IsEnabled = false;
            andQuery_button.IsEnabled = false;
        }

        //----< set client path for checkedout file  >------
        private void SelectPath_Button_Click(object sender, RoutedEventArgs e)
        {
            saveFileDialog.FileName = "FolderSelection";

            if (saveFileDialog.ShowDialog() == true)
            {
                savePath = saveFileDialog.FileName;
                savePath = modifySavePath(savePath);
            }
        }

        //----< request repository for files to select children  >----------------
        private void AddChildren_Button_Click(object sender, RoutedEventArgs e)
        {
            children_button.IsEnabled = false;
            RepoStorage Dir = new RepoStorage("Storage");
            root = Dir;
            rs = root;
            CsMessage msg = new CsMessage();
            msg.add("to", CsEndPoint.toString(serverEndPoint));
            msg.add("from", CsEndPoint.toString(endPoint_));
            msg.add("command", "getRepoStorage");
            msg.add("path", "../Storage");
            msg.add("parentDir", "Storage");
            try
            {
                translater.postMessage(msg);
            }
            catch (Exception)
            {
                Console.WriteLine("Unable to send message...");
            }

        }

        //----< get parent directory >----------------
        private void getParent(RepoStorage RS, string dir)
        {
            if (RS.Dir_ == dir)
            {
                rs = RS;
                return;
            }
            foreach (RepoStorage child in RS.subDirs_)
            {
                getParent(child, dir);
            }
        }

        //----< synchronize filelist and key list >----------------
        private void FileList_SelectionChanged(object sender, SelectionChangedEventArgs e)
        {
            KeyList.SelectedIndex = FileList.SelectedIndex;
        }

        //----< synchronize filelist and key list >----------------
        private void KeyList_SelectionChanged(object sender, SelectionChangedEventArgs e)
        {
            FileList.SelectedIndex = KeyList.SelectedIndex;
        }

        //----< synchronize filelist and key list scrollbar>----------------
        private void keys_ScrollChanged(object sender, ScrollChangedEventArgs e)
        {
            ScrollViewer _listboxScrollViewer1 = GetDescendantByType(KeyList, typeof(ScrollViewer)) as ScrollViewer;
            ScrollViewer _listboxScrollViewer2 = GetDescendantByType(FileList, typeof(ScrollViewer)) as ScrollViewer;
            _listboxScrollViewer2.ScrollToVerticalOffset(_listboxScrollViewer1.VerticalOffset);
        }

        //----< helper to synchronize filelist and key list >----------------
        public Visual GetDescendantByType(Visual element, Type type)
        {
            if (element == null) return null;
            if (element.GetType() == type) return element;
            Visual foundElement = null;
            if (element is FrameworkElement)
            {
                (element as FrameworkElement).ApplyTemplate();
            }
            for (int i = 0; i < VisualTreeHelper.GetChildrenCount(element); i++)
            {
                Visual visual = VisualTreeHelper.GetChild(element, i) as Visual;
                foundElement = GetDescendantByType(visual, type);
                if (foundElement != null)
                    break;
            }
            return foundElement;
        }

        //----< sets the scroll for keylist and filelist >----------------
        private void files_ScrollChanged(object sender, ScrollChangedEventArgs e)
        {
            ScrollViewer _listboxScrollViewer1 = GetDescendantByType(KeyList, typeof(ScrollViewer)) as ScrollViewer;
            ScrollViewer _listboxScrollViewer2 = GetDescendantByType(FileList, typeof(ScrollViewer)) as ScrollViewer;
            _listboxScrollViewer1.ScrollToVerticalOffset(_listboxScrollViewer2.VerticalOffset);
        }

        //----< store current metadata and clear all fields >----------------
        private void addAnotherFile_Button_Click(object sender, RoutedEventArgs e)
        {
            if (selectedFile == "" || FileNameBox.Text == "")
            {
                statusBarText.Text += "\n Could not add file for checking in: No file selected...";
                return;
            }
            CIFD.Add(new checkInFileData(selectedFile, selectedFilePath, FileNameBox.Text, DescripBox.Text, CategoryBox.Text, StatusBox.SelectionBoxItem.ToString(), children));
            clearCheckInFields();
        }

        //----< clear all checkin fields >----------------
        private void clearCheckInFields()
        {
            selectedFile = "";
            selectedFilePath = "";
            children = "";
            FileNameBox.Clear();
            DescripBox.Clear();
            CategoryBox.Clear();
        }

        //----< query the database with given attributes >----------------
        private void searchDatabase_Button_Click(object sender, RoutedEventArgs e)
        {
            int version;
            bool parsed = Int32.TryParse(versionQuery_txt.Text, out version);
            if (!parsed && versionQuery_txt.Text != ""){
                QD.Clear();
                statusBarText.Text += "\n Could not search database: Incorrect version format...";
                return;
            }
            clearDirs();
            clearFiles();
            pathStack_.Clear();
            int count = 0;
            string noDependantsCheck = "false";
            if (noDependants.IsChecked == true)
                noDependantsCheck = "true";
            QD.Add(new queryData(filenameQuery_txt.Text, categoryQuery_txt.Text, dependencyQuery_txt.Text, versionQuery_txt.Text, noDependantsCheck));
            CsMessage msg = new CsMessage();
            msg.add("to", CsEndPoint.toString(serverEndPoint));
            msg.add("from", CsEndPoint.toString(endPoint_));
            msg.add("command", "searchDatabase");
            foreach (var Q in QD){
                count++;
                msg.add("filename" + count.ToString(), Q.filename_);
                msg.add("dependencies" + count.ToString(), Q.dependencies_);
                msg.add("category" + count.ToString(), Q.category_);
                msg.add("version" + count.ToString(), versionQuery_txt.Text);
                msg.add("nodependants" + count.ToString(), Q.noDependants_);
            }
            msg.add("count", count.ToString());
            if (unionQuery_button.IsEnabled && andQuery_button.IsEnabled)
                msg.add("querytype", "single");
            else if (unionQuery_button.IsEnabled)
                msg.add("querytype", "union");
            else
                msg.add("querytype", "and");
            try{
                translater.postMessage(msg);
            }
            catch (Exception){
                Console.WriteLine("Unable to send message...");
            }
            cancelSearch_button.IsEnabled = true;
            searchDatabase_button.IsEnabled = false;
            QD.Clear();
            andQuery_button.IsEnabled = false;
            unionQuery_button.IsEnabled = false;
        }

        //----< cancel query to database and restore directory structure >----------------
        private void cancelSearch_Button_Click(object sender, RoutedEventArgs e)
        {
            clearDirs();
            clearFiles();
            pathStack_.Push("../Storage");
            populateDirsFiles();
            cancelSearch_button.IsEnabled = false;
            searchDatabase_button.IsEnabled = true;
            QD.Clear();
            andQuery_button.IsEnabled = true;
            unionQuery_button.IsEnabled = true;
        }

        //----< store current query atttributes and union another query >----------------
        private void unionQuery_Button_Click(object sender, RoutedEventArgs e)
        {
            int version;
            bool parsed = Int32.TryParse(versionQuery_txt.Text, out version);
            if (!parsed && versionQuery_txt.Text != "")
            {
                QD.Clear();
                statusBarText.Text += "\n Could not search database: Incorrect version format...";
                return;
            }
            string noDependantsCheck = "false";
            if (noDependants.IsChecked == true)
                noDependantsCheck = "true";
            QD.Add(new queryData(filenameQuery_txt.Text, categoryQuery_txt.Text, dependencyQuery_txt.Text, versionQuery_txt.Text, noDependantsCheck));
            filenameQuery_txt.Clear();
            dependencyQuery_txt.Clear();
            categoryQuery_txt.Clear();
            versionQuery_txt.Clear();
            noDependants.IsChecked = false;
            andQuery_button.IsEnabled = false;
        }

        //----< store current query atttributes and 'and' another query >----------------
        private void andQuery_Button_Click(object sender, RoutedEventArgs e)
        {
            int version;
            bool parsed = Int32.TryParse(versionQuery_txt.Text, out version);
            if (!parsed && versionQuery_txt.Text != "")
            {
                QD.Clear();
                statusBarText.Text += "\n Could not search database: Incorrect version format...";
                return;
            }
            string noDependantsCheck = "false";
            if (noDependants.IsChecked == true)
                noDependantsCheck = "true";
            QD.Add(new queryData(filenameQuery_txt.Text, categoryQuery_txt.Text, dependencyQuery_txt.Text, versionQuery_txt.Text, noDependantsCheck));
            filenameQuery_txt.Clear();
            dependencyQuery_txt.Clear();
            categoryQuery_txt.Clear();
            versionQuery_txt.Clear();
            unionQuery_button.IsEnabled = false;
        }
    }

    //----< struct to store repo storage directory structure for adding children >----------------
    public struct RepoStorage
    {
        public string Dir_ { get; set; }
        public List<string> files_ { get; set; }
        public List<RepoStorage> subDirs_ { get; set; }
        public RepoStorage(string dir)
        {
            Dir_ = dir;
            files_ = new List<string>();
            subDirs_ = new List<RepoStorage>();
        }
    }

    //----< struct to store checkin metadata for multiple checkin >----------------
    public struct checkInFileData
    {
        public string file_ { get; set; }
        public string filepath_ { get; set; }
        public string filename_ { get; set; }
        public string description_ { get; set; }
        public string category_ { get; set; }
        public string status_ { get; set; }
        public string childs_ { get; set; }
        public checkInFileData(string selectedFile, string selectedFilePath, string FileNameBox, string DescripBox, string CategoryBox, string StatusBox, string children)
        {
            file_ = selectedFile;
            filepath_ = selectedFilePath;
            filename_ = FileNameBox;
            description_ = DescripBox;
            category_ = CategoryBox;
            status_ = StatusBox;
            childs_ = children;
        }
    }

    //----< struct to store quey attributes for adding more queries >----------------
    public struct queryData
    {
        public string filename_ { get; set; }
        public string category_ { get; set; }
        public string dependencies_ { get; set; }
        public string version_ { get; set; }
        public string noDependants_ { get; set; }
        public queryData(string filename, string category, string dependencies, string version, string noDependants)
        {

            filename_ = filename;
            category_ = category;
            dependencies_ = dependencies;
            version_ = version;
            noDependants_ = noDependants;
        }
    }
}
