using System.ComponentModel.DataAnnotations;

namespace CanadaWalksAPI.Models.DTO
{
    public class UpdateRegionDTO
    {
        [MinLength(1, ErrorMessage = "Code needs to be atleast 1 character")]
        [MaxLength(3, ErrorMessage = "Code needs to be less than 2 characters")]
        public string? Code { get; set; } // Code representing the region (e.g., "ON" for Ontario)
        public string? Name { get; set; } // Name of the region (e.g., "Ontario")
        public string? RegionImageUrl { get; set; } // URL of the image representing the region, can be null
    }
}
