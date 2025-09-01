using CanadaWalksAPI.Models.Domain;

namespace CanadaWalksAPI.Models.DTO
{
    public class UpdateWalkDTO
    {
        public string? Name { get; set; }
        public string? Description { get; set; }
        public double? Length { get; set; } // Length in kilometers
        public string? WalkImageUrl { get; set; } // URL of the image representing the walk
        public Guid? RegionId { get; set; } // Foreign key to the Region
        public Guid? DifficultyId { get; set; } // Foreign key to the Difficulty
    }
}
