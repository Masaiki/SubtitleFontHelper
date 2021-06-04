using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Threading;

namespace SubtitleFontHelper
{
    class GlobalContext
    {
        public static FontMatcher FontMatcher { get; set; } = new FontMatcher();
        public static Thread ServerThread { get; set; }
        public static NamedPipeServer NamedPipeServer { get; set; }
    }
}
