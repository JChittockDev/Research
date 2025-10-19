using CanadaWalksAPI.Models.DTO;
using Microsoft.AspNetCore.Mvc;
using System.Threading.Tasks;

namespace CanadaWalksAPI.UI.Controllers
{
    public class RegionsController : Controller
    {
        private readonly IHttpClientFactory httpClientFactory;

        public RegionsController(IHttpClientFactory httpClientFactory)
        {
            this.httpClientFactory = httpClientFactory;
        }
        public async Task<IActionResult> Index()
        {
            List<RegionDTO> regions = new List<RegionDTO>();

            try
            {
                // Get all regions from web API

                var client = httpClientFactory.CreateClient("CanadaWalksAPI");

                var response = await client.GetAsync("/api/regions");
                response.EnsureSuccessStatusCode();

                regions.AddRange(await response.Content.ReadFromJsonAsync<IEnumerable<RegionDTO>>());
            }
            catch (Exception ex)
            {
                // Log the exception (you can use a logging framework here)
                Console.WriteLine($"An error occurred: {ex.Message}");
                // Optionally, you can set a user-friendly error message to display in the view
                ViewBag.ErrorMessage = "An error occurred while fetching regions. Please try again later.";
            }

            return View(regions);
        }
    }
}
