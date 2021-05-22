using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace SubtitleFontHelper
{
    class RecursiveFileEnumerator
    {
        private object _lock = new object();

        private Stack<IEnumerator<FileSystemInfo>> _stack = new Stack<IEnumerator<FileSystemInfo>>();

        public delegate void DirectoryAccessExceptionHandler(Object sender, DirectoryInfo directory,
            Exception exception);

        public event DirectoryAccessExceptionHandler OnDirectoryAccessException = delegate { };

        public RecursiveFileEnumerator(DirectoryInfo[] directory)
        {
            foreach (var dirInfo in directory.Reverse())
            {
                FileSystemInfo[] fsInfos;
                try
                {
                    fsInfos = dirInfo.GetFileSystemInfos();
                }
                catch (Exception e)
                {
                    OnDirectoryAccessException(this, dirInfo, e);
                    continue;
                }
                IEnumerable<FileSystemInfo> iter = (IEnumerable<FileSystemInfo>)fsInfos;
                _stack.Push(iter.GetEnumerator());
            }
        }

        private FileInfo NextFile()
        {
            while (_stack.Count != 0)
            {
                bool hasNext = _stack.Peek().MoveNext();
                if (hasNext)
                {
                    FileSystemInfo fsInfo = _stack.Peek().Current;
                    if (fsInfo is DirectoryInfo)
                    {
                        DirectoryInfo dirInfo = fsInfo as DirectoryInfo;
                        FileSystemInfo[] fsInfos;
                        try
                        {
                            fsInfos = dirInfo.GetFileSystemInfos();
                        }
                        catch (Exception e)
                        {
                            OnDirectoryAccessException(this, dirInfo, e);
                            continue;
                        }

                        IEnumerable<FileSystemInfo> iter = (IEnumerable<FileSystemInfo>)fsInfos;
                        _stack.Push(iter.GetEnumerator());
                    }
                    else if (fsInfo is FileInfo)
                    {
                        FileInfo fileInfo = fsInfo as FileInfo;
                        return fileInfo;
                    }
                    else
                    {
                        continue;
                    }
                }
                else
                {
                    _stack.Pop();
                }
            }
            return null;
        }

        public FileInfo NextFileLocked()
        {
            lock (_lock) return NextFile();
        }

        public FileInfo NextFileExtLocked(IList<string> extensions)
        {
            FileInfo fileInfo = NextFileLocked();

            while (fileInfo != null)
            {
                foreach (var ext in extensions)
                {
                    if (fileInfo.Extension.Equals(ext, StringComparison.OrdinalIgnoreCase))
                    {
                        return fileInfo;
                    }
                }

                fileInfo = NextFileLocked();
            }

            return null;
        }
    }
}
