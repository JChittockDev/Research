using CanadaWalksAPI.Models.Domain;

namespace CanadaWalksAPI.Models.DTO
{
    public class WalkDTO
    {
        public Guid Id { get; set; }
        public string Name { get; set; }
        public string Description { get; set; }
        public double Length { get; set; } // Length in kilometers
        public string? WalkImageUrl { get; set; } // URL of the image representing the walk
        public RegionDTO Region { get; set; } // Navigation property to the Region
        public DifficultyDTO Difficulty { get; set; } // Navigation property to the Difficulty
    }
}
