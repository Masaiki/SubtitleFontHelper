using System;
using System.Collections.Generic;
using System.Linq;
using System.Runtime.InteropServices;
using System.Text;
using System.Threading.Tasks;

namespace SubtitleFontHelper
{
    class Utility
    {
        [DllImport("ntdll.dll", PreserveSig = false)]
        public static extern long NtSuspendProcess(IntPtr processHandle);
    }
}
