using System.ComponentModel.DataAnnotations;

namespace CanadaWalksAPI.Models.DTO
{
    public class AddImageDTO
    {
        [Required]
        public IFormFile File { get; set; }
        [Required]
        public string FileName { get; set; }
        public string? Description { get; set; }

    }
}
