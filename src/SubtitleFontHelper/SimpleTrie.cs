using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace SubtitleFontHelper
{
    class SimpleTrie<T,U>
    {
        private class TrieNode
        {
            public Dictionary<T, TrieNode> Children { get; set; } = new Dictionary<T, TrieNode>();
            public T Letter { get; set; }
            public U Data { get; set; }
        }

        TrieNode _root = new TrieNode();

        public bool InsertString(IEnumerable<T> str, U value)
        {
            TrieNode node = _root;
            foreach(var ch in str)
            {
                TrieNode next;
                if (!node.Children.TryGetValue(ch, out next))
                {
                    next = new TrieNode();
                    next.Letter = ch;
                    node.Children.Add(ch, next);
                    node = next;
                }
                else
                {
                    node = next;
                }
            }
            if (node.Data != null)
            {
                return false;
            }
            else
            {
                node.Data = value;
                return true;
            }
        }

        /// <summary>
        /// Find a string in Trie
        /// </summary>
        /// <param name="str">The string</param>
        /// <param name="value">The result. 
        /// In case of str is a prefix or str is found, the value will be set to the data of the first complete string.
        /// Or it should be default.
        /// </param>
        /// <returns>Is the search succeed</returns>
        public bool FindString(IEnumerable<T> str, out U value)
        {
            TrieNode node = _root;
            value = default;
            foreach(var ch in str)
            {
                TrieNode next;
                if (!node.Children.TryGetValue(ch, out next))
                {
                    return false;
                }
                else
                {
                    node = next;
                }
            }
            if (node.Data == null)
            {
                var iter = node.Children.GetEnumerator();
                while (iter.MoveNext())
                {
                    node = iter.Current.Value;
                    iter = node.Children.GetEnumerator();
                    if (node.Data != null)
                    {
                        value = node.Data;
                        break;
                    }
                }
                return false;
            }
            else
            {
                value = node.Data;
                return true;
            }
            
        }
    }
}
