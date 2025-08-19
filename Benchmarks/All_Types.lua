-- Lua 5.4 - Consolidated Code Example

-- 1. Variables and basic operations
print("-- 1. Variables and basic operations --")
local message = "Hello from Lua 5.4!"
local number_value = 10
local boolean_value = true

print(message)
print("The number is:", number_value)
print("Is it true?", boolean_value)

local sum = 5 + 3
local product = 4 * 2
local difference = 10 - 7
local division = 15 / 3
local power = 2 ^ 4
local modulo = 7 % 3

print("Sum:", sum)
print("Product:", product)
print("Difference:", difference)
print("Division:", division)
print("Power:", power)
print("Modulo:", modulo)

local first_name = "John"
local last_name = "Doe"
local full_name = first_name .. " " .. last_name
print("Full Name:", full_name)

-- 2. Conditional statements
print("\n-- 2. Conditional statements --")
local temperature = 25

if temperature > 30 then
  print("It's a hot day!")
elseif temperature >= 20 and temperature <= 30 then
  print("It's a pleasant day.")
else
  print("It's a bit chilly.")
end

local is_raining = false
local has_umbrella = true

if is_raining and has_umbrella then
  print("You're covered!")
elseif is_raining and not has_umbrella then
  print("You might get wet.")
else
  print("No need for an umbrella.")
end

-- 3. Loops
print("\n-- 3. Loops --")

-- For loop (numerical)
print("\n--- For Loop ---")
for i = 1, 5 do
  print("Count:", i)
end

-- While loop
print("\n--- While Loop ---")
local counter = 0
while counter < 3 do
  print("While loop iteration:", counter)
  counter = counter + 1
end

-- Repeat-until loop
print("\n--- Repeat-Until Loop ---")
local number_to_guess = 7
local guess = 0

repeat
  print("Guess the number (1-10):")
  -- In a real scenario, you'd want to handle invalid input (non-numbers)
  -- For this example, we assume valid number input or an error will occur
  local input = io.read()
  guess = tonumber(input)

  if guess == nil then
    print("Invalid input. Please enter a number.")
  elseif guess < number_to_guess then
    print("Too low!")
  elseif guess > number_to_guess then
    print("Too high!")
  end
until guess == number_to_guess

print("Congratulations! You guessed it!")

-- 4. Functions
print("\n-- 4. Functions --")

function greet(name)
  print("Hello, " .. name .. "!")
end

greet("Alice")

function add(a, b)
  return a + b
end

local result = add(5, 7)
print("Sum:", result)

function get_coords()
  return 10, 20
end

local x, y = get_coords()
print("X:", x, "Y:", y)

-- 5. Tables (arrays and dictionaries)
print("\n-- 5. Tables (arrays and dictionaries) --")

-- Array-like table
local colors = {"red", "green", "blue"}
print("First color:", colors[1])
print("Last color:", colors[3])

-- Inserting into a table
table.insert(colors, "yellow")
print("New last color:", colors[4])

-- Dictionary-like table
local person = {name = "Bob", age = 30, city = "New York"}
print("Person's name:", person.name)
print("Person's age:", person["age"])

-- Adding/modifying elements
person.occupation = "Engineer"
person.city = "San Francisco"

print("Person's occupation:", person.occupation)
print("Person's updated city:", person.city)

-- Iterating over a table (dictionary-like)
print("\n--- Iterating over 'person' table ---")
for key, value in pairs(person) do
  print(key, value)
end

-- Iterating over an array-like table
print("\n--- Iterating over 'colors' table (array-like) ---")
for i, color in ipairs(colors) do
  print(i, color)
end
