using CanadaWalksAPI.Models.Domain;
using System.ComponentModel.DataAnnotations;

namespace CanadaWalksAPI.Models.DTO
{
    public class AddWalkDTO
    {
        [Required]
        public string Name { get; set; }
        [Required]
        public string Description { get; set; }
        [Required]
        public double Length { get; set; } // Length in kilometers
        public string? WalkImageUrl { get; set; } // URL of the image representing the walk
        [Required]
        public Guid RegionId { get; set; } // Foreign key to the Region
        [Required]
        public Guid DifficultyId { get; set; } // Foreign key to the Difficulty
    }
}
