using System.ComponentModel.DataAnnotations.Schema;

namespace CanadaWalksAPI.Models.DTO
{
    public class ImageDTO
    {
        public Guid Id { get; set; }
        public string FileName { get; set; }
        public string? Description { get; set; }
        public string FileType { get; set; }
        public long FileSize { get; set; }
        public string FilePath { get; set; }
    }
}
