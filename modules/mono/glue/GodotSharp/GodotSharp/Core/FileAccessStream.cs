using System;
using System.IO;

namespace Godot
{
    /// <summary>
    /// Provides a stream implementation that wraps around the Godot FileAccess class.
    /// Supports reading from, writing to, and seeking within a file.
    /// </summary>
    public class FileAccessStream : Stream
    {
        private readonly FileAccess _fileAccess;
        private readonly long _fileLength;
        private long _position;

        /// <summary>
        /// See <see cref="FileAccess.ModeFlags"/>
        /// </summary>
        public FileAccess.ModeFlags ModeFlags { get; private init; }

        /// <summary>
        /// Initializes a new instance of the <see cref="FileAccessStream"/> class.
        /// Opens the file at the specified path using the given mode flags.
        /// </summary>
        public FileAccessStream(FileAccess fileAccess, FileAccess.ModeFlags modeFlags)
        {
            _fileAccess = fileAccess;
            _fileLength = (long)_fileAccess.GetLength();
            _position = 0;
            ModeFlags = modeFlags;
        }

        /// <summary>
        /// Gets a value indicating whether the current stream supports reading.
        /// Always returns true because FileAccess supports reading.
        /// </summary>
        public override bool CanRead => (ModeFlags & FileAccess.ModeFlags.Read) != 0;

        /// <summary>
        /// Gets a value indicating whether the current stream supports seeking.
        /// Always returns true because FileAccess supports seeking.
        /// </summary>
        public override bool CanSeek => true;

        /// <summary>
        /// Gets a value indicating whether the current stream supports writing.
        /// Returns true if the FileAccess mode includes writing.
        /// </summary>
        public override bool CanWrite => (ModeFlags & FileAccess.ModeFlags.Write) != 0;

        /// <summary>
        /// Gets the length of the file stream in bytes.
        /// </summary>
        public override long Length => _fileLength;

        /// <summary>
        /// Gets or sets the position within the current stream.
        /// </summary>
        public override long Position
        {
            get => _position;
            set => Seek(value, SeekOrigin.Begin);
        }

        /// <summary>
        /// Flushes any buffered data to the file.
        /// Calls the FileAccess.Flush method.
        /// </summary>
        public override void Flush()
        {
            _fileAccess.Flush();
        }

        /// <summary>
        /// Reads a sequence of bytes from the current stream and advances the position within the stream by the number of bytes read.
        /// </summary>
        /// <param name="buffer">An array of bytes. When this method returns, the buffer contains the specified byte array with the values between offset and (offset + count - 1) replaced by the bytes read from the current source.</param>
        /// <param name="offset">The zero-based byte offset in buffer at which to begin storing the data read from the current stream.</param>
        /// <param name="count">The maximum number of bytes to be read from the current stream.</param>
        /// <returns>The total number of bytes read into the buffer. This can be less than the number of bytes requested if that many bytes are not currently available, or zero if the end of the stream has been reached.</returns>
        public override int Read(byte[] buffer, int offset, int count)
        {
            if (_position >= _fileLength) return 0;

            byte[] chunk = _fileAccess.GetBuffer(count);

            Buffer.BlockCopy(chunk, 0, buffer, offset, chunk.Length);
            _position += chunk.Length;

            return chunk.Length;
        }

        /// <summary>
        /// Sets the position within the current stream.
        /// </summary>
        /// <param name="offset">A byte offset relative to the origin parameter.</param>
        /// <param name="origin">A value of type SeekOrigin indicating the reference point used to obtain the new position.</param>
        /// <returns>The new position within the current stream.</returns>
        public override long Seek(long offset, SeekOrigin origin)
        {
            var newPosition = origin switch
            {
                SeekOrigin.Begin => offset,
                SeekOrigin.Current => _position + offset,
                SeekOrigin.End => _fileLength + offset,
                _ => throw new ArgumentException("Invalid seek origin.", nameof(origin)),
            };

            _fileAccess.Seek((ulong)newPosition);
            _position = newPosition;

            return _position;
        }

        /// <summary>
        /// Sets the length of the current stream.
        /// Always throws NotSupportedException because FileAccessStream does not support setting the length.
        /// </summary>
        /// <param name="value">The desired length of the current stream in bytes.</param>
        /// <exception cref="NotSupportedException">Always thrown.</exception>
        public override void SetLength(long value) => throw new NotSupportedException();

        /// <summary>
        /// Writes a sequence of bytes to the current stream and advances the current position within this stream by the number of bytes written.
        /// </summary>
        /// <param name="buffer">An array of bytes. This method copies count bytes from buffer to the current stream.</param>
        /// <param name="offset">The zero-based byte offset in buffer at which to begin copying bytes to the current stream.</param>
        /// <param name="count">The number of bytes to be written to the current stream.</param>
        public override void Write(byte[] buffer, int offset, int count)
        {
            byte[] chunk = new byte[count];
            Buffer.BlockCopy(buffer, offset, chunk, 0, count);

            _fileAccess.StoreBuffer(chunk);
            _position += count;
        }
    }

    /// <summary>
    ///
    /// </summary>
    public static class FileAccessExtensions
    {
        /// <summary>
        ///
        /// </summary>
        /// <param name="fileAccess"></param>
        /// <param name="modeFlags"></param>
        /// <returns></returns>
        public static FileAccessStream GetStream(this FileAccess fileAccess, FileAccess.ModeFlags modeFlags)
        {
            return new(fileAccess, modeFlags);
        }
    }
}
