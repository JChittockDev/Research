namespace CanadaWalksAPI.Models.Domain
{
    public class Region
    {
        public Guid Id { get; set; }
        public string Code { get; set; }
        public string Name { get; set; }
        // The URL of the image representing the region ? means it can accept null values
        public string? RegionImageUrl { get; set; }
    } 
}
