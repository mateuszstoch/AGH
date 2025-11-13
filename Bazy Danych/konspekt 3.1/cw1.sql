-- zadanie 1
select o.OrderID ,sum(od.Quantity) , c.CompanyName  from Orders o 
inner join [Order Details] od on od.OrderID = o.OrderID 
inner join Customers c on c.CustomerID = o.CustomerID 
group by o.OrderID ,c.CompanyName 

-- zadanie 2
select o.OrderID ,sum(od.Quantity) , c.CompanyName  from Orders o 
inner join [Order Details] od on od.OrderID = o.OrderID 
inner join Customers c on c.CustomerID = o.CustomerID 
group by o.OrderID ,c.CompanyName 
having sum(od.Quantity) > 250

-- zadanie 3
select o.OrderID ,round(sum(od.Quantity*od.UnitPrice *(1- od.Discount)),2) , c.CompanyName  from Orders o 
inner join [Order Details] od on od.OrderID = o.OrderID 
inner join Customers c on c.CustomerID = o.CustomerID 
group by o.OrderID ,c.CompanyName 

-- zadanie 4
select o.OrderID ,round(sum(od.Quantity*od.UnitPrice *(1- od.Discount)),2) , c.CompanyName  from Orders o 
inner join [Order Details] od on od.OrderID = o.OrderID 
inner join Customers c on c.CustomerID = o.CustomerID 
group by o.OrderID ,c.CompanyName 
having sum(od.Quantity)>250

-- zadanie 5
select o.OrderID ,round(sum(od.Quantity*od.UnitPrice *(1- od.Discount)),2) , c.CompanyName,e.FirstName ,e.LastName   from Orders o 
inner join [Order Details] od on od.OrderID = o.OrderID 
inner join Customers c on c.CustomerID = o.CustomerID 
inner join Employees e on e.EmployeeID  = o.EmployeeID 
group by o.OrderID ,c.CompanyName ,e.FirstName ,e.LastName 
having sum(od.Quantity)>250