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

        #region Singleton
        private static GlobalContext _instance;
        private static readonly object _lock = new object();

        public static GlobalContext Instance
        {
            get
            {
                if (_instance == null)
                {
                    lock (_lock)
                    {
                        if (_instance == null)
                        {
                            _instance = new GlobalContext();
                        }
                    }
                }
                return _instance;
            }
        }
        #endregion

        private FontIndex _fontindex = new FontIndex();

        public FontIndex FontIndex => _fontindex;

        public Thread ServerThread { get; set; }
        public NamedPipeServer NamedPipeServer { get; set; }

        private GlobalContext()
        {

        }


    }
}
