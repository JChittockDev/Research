namespace CanadaWalksAPI.Models.DTO
{
    public class UpdateRegionDTO
    {
        public string? Code { get; set; } // Code representing the region (e.g., "ON" for Ontario)
        public string? Name { get; set; } // Name of the region (e.g., "Ontario")
        public string? RegionImageUrl { get; set; } // URL of the image representing the region, can be null
    }
}
