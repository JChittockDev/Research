namespace CanadaWalksAPI.Models.Domain
{
    public class Walk
    {
        public Guid Id { get; set; }
        public string Name { get; set; }
        public string Description { get; set; }
        public double Length { get; set; } // Length in kilometers
        public string? WalkImageUrl { get; set; } // URL of the image representing the walk
        public Guid RegionId { get; set; } // Foreign key to the Region
        public Region Region { get; set; } // Navigation property to the Region
        public Guid DifficultyId { get; set; } // Foreign key to the Difficulty
        public Difficulty Difficulty { get; set; } // Navigation property to the Difficulty
    }
}
