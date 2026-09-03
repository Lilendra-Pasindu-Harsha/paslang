# PasLang Mobile & Web Application Logic Engine Demo
# Demonstrates Object-Oriented Classes, Data Maps, Arrays, and Full-Stack Business Logic.

say "=================================================="
say "   PasLang Mobile & Web App Logic Engine Demo     "
say "=================================================="
say ""

# 1. Define User Account Class
class UserAccount:
    function get_balance(initial_deposit):
        return add initial_deposit 100

# 2. Process User Profile Data Map
let user_profile = {
    name: "Alex Johnson",
    role: "Lead Engineer",
    active: true
}

say "User Profile Data:"
say user_profile
say ""

# 3. Calculate Financial / Metrics Analytics
let monthly_sales = [1200, 1450, 1900, 2100, 2500]
say "Monthly Sales History:"
say monthly_sales

let total_sales = 0
for sale in monthly_sales:
    total_sales = add total_sales sale

say "Total Revenue:"
say total_sales

let num_months = len monthly_sales
let avg_sale = div total_sales num_months
say "Average Monthly Revenue:"
say avg_sale
say ""

say "Web & Mobile App Engine Demo complete!"
