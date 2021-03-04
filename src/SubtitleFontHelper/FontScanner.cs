using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

using libSubtitleFontHelper;

namespace SubtitleFontHelper
{
    public class FontScanner
    {
        private string _current_file = "N/A";
        private int _progress = 0;

        private object _status_lock = new object();

        private FreeTypeFontMetadata _metadata = new FreeTypeFontMetadata();

        public Tuple<string, int> GetStatusTuple()
        {
            lock (_status_lock)
            {
                return new Tuple<string, int>(_current_file, _progress);
            }
        }

        /// <summary>
        /// Scan a directory, return a list of file
        /// </summary>
        /// <param name="directory">directory to be scanned</param>
        /// <param name="suffixes">filename suffixes</param>
        /// <returns>file list</returns>
        public static List<string> ScanDirectory(string directory, IEnumerable<string> suffixes)
        {
            List<string> file_list = new List<string>();

            void TraverseDirectory(DirectoryInfo dir, IEnumerable<string> suffix)
            {
                FileInfo[] files;
                DirectoryInfo[] directories;
                try
                {
                    files = dir.GetFiles();
                    foreach (var file in files)
                    {
                        string filename = file.FullName;
                        foreach (var suf in suffix)
                        {
                            if (filename.EndsWith(suf, StringComparison.OrdinalIgnoreCase))
                            {
                                file_list.Add(filename);
                                break;
                            }
                        }
                    }
                }
                catch (PathTooLongException) { }
                catch (DirectoryNotFoundException) { }

                try
                {
                    directories = dir.GetDirectories();
                    foreach (var d in directories)
                    {
                        TraverseDirectory(d, suffix);
                    }
                }
                catch (DirectoryNotFoundException) { }
                catch (System.Security.SecurityException) { }
                catch (UnauthorizedAccessException) { }
            }

            DirectoryInfo directoryInfo = new DirectoryInfo(directory);
            TraverseDirectory(directoryInfo, suffixes);

            return file_list;
        }


        public FontFileCollection ScanFont(IEnumerable<string> files)
        {
            FontFileCollection collection = new FontFileCollection();
            int files_count = files.Count();
            for (int n = 0; n < files_count; ++n)
            {
                string file = files.ElementAt(n);
                lock (_status_lock)
                {
                    _current_file = file;
                    _progress = n + 1;
                }

                FontFile fontFile = new FontFile();
                fontFile.FileName = file;
                _metadata.OpenFontFile(file);
                int face_count = _metadata.GetFontFaceCount();
                for(int i = 0; i < face_count; ++i)
                {
                    IFreeTypeFontFaceMetadata faceMetadata = _metadata.GetFontFace(i);
                    FontFace face = FontFace.FromMetadata(faceMetadata);
                    face.FontFaceIndex = i;
                    fontFile.FontFaces.Add(face);
                }
                if (fontFile.FontFaces.Count != 0) collection.FontFiles.Add(fontFile);
            }
            return collection;
        }

    }
}
