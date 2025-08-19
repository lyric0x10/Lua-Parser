function factorial(n)
  if n == 0 then
    return 1
  else
    return n * factorial(n - 1)
  end
end

local num = 5
print("Factorial of", num, "is", factorial(num))
