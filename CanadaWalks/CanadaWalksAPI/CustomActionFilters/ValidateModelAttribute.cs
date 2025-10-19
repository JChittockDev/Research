using Microsoft.AspNetCore.Mvc;
using Microsoft.AspNetCore.Mvc.Filters;

// Custom action filter to validate the model state
// The model state is a collection of name and value pairs that are submitted to the server during a POST request
// It validates the data against the model's data annotations and other validation rules that are defined in the model class
namespace CanadaWalksAPI.CustomActionFilters
{
    // This attribute can be applied to controller actions to automatically validate the model state
    public class ValidateModelAttribute : ActionFilterAttribute
    {
        // This method is called before the action method is executed
        public override void OnActionExecuting(ActionExecutingContext context)
        {
            // If the model state is not valid, return a 400 Bad Request response with the model state errors
            if (!context.ModelState.IsValid)
            {
                context.Result = new BadRequestObjectResult(context.ModelState);
            }
        }
    }
}
