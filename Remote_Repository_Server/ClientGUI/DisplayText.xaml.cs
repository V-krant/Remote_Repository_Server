///////////////////////////////////////////////////////////////////////
// DisplayText.xaml.cs - Popup Window to display file text           //
// ver 1.0                                                           //
// Vikrant Bhopatrao, CSE687 - Object Oriented Design, Spring 2018   //
///////////////////////////////////////////////////////////////////////
/*
 * Package Operations:
 * -------------------
 * This package provides a window containing a text block to display the 
 * contents of a file requested by the client
 *   
 * Required Files:
 * ---------------
 * DisplayText.xaml.cs, DisplayText.xaml
 * 
 * Maintenance History:
 * --------------------
 * ver 1.0 : 9 April 2018
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
    /// Interaction logic for DisplayText.xaml
    /// </summary>
    public partial class DisplayText : Window
    {
        public DisplayText()
        {
            InitializeComponent();
        }
    }
}
