using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Input;

namespace SubtitleFontHelper.Commands
{
    public static class Commands
    {
        public static readonly RoutedCommand Exit = new RoutedCommand("Exit", typeof(Commands));
    }
}
