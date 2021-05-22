using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Threading;

namespace SubtitleFontHelper
{
    class FontIndex
    {
        private SimpleTrie<char, FontFile> _trie = new SimpleTrie<char, FontFile>();

        readonly private ReaderWriterLockSlim _rwlock = new ReaderWriterLockSlim();

        public void AddFont(FontFile file)
        {
            foreach(var face in file.FontFaces)
            {
                foreach(var fullname in face.FullNames)
                {
                    _trie.InsertString(fullname, file);
                }
            }
        }

        public void AddFontLocked(FontFile file)
        {
            _rwlock.EnterWriteLock();
            AddFont(file);
            _rwlock.ExitWriteLock();
        }

        public FontFile FindFont(string fullname)
        {
            FontFile ret;
            _trie.FindString(fullname, out ret);
            return ret;
        }

        public FontFile FindFontLocked(string fullname)
        {
            FontFile ret;
            _rwlock.EnterReadLock();
            ret = FindFont(fullname);
            _rwlock.ExitReadLock();
            return ret;
        }

        
    }
}
