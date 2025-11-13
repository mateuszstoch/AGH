-- zadanie 1
select s.CompanyName ,count(*) from Shippers s inner join Orders o 
on o.ShipVia = s.ShipperID and year(o.ShippedDate) = '1997' 
group by s.ShipperID,s.CompanyName  

-- zadanie 2
select top 1 s.CompanyName ,count(*) from Shippers s 
inner join Orders o on o.ShipVia = s.ShipperID and year(o.ShippedDate ) = '1997' 
group by s.ShipperID,s.CompanyName
order by count(*) desc

-- zadanie 3
select e.FirstName ,e.LastName ,count(*) from Employees e 
inner join orders o on o.EmployeeID = e.EmployeeID 
group by e.EmployeeID ,e.FirstName ,e.LastName 

-- zadanie 4
select e.FirstName ,e.LastName ,count(*) from Employees e 
inner join orders o on o.EmployeeID = e.EmployeeID and year(o.OrderDate) = '1997'
group by e.EmployeeID ,e.FirstName ,e.LastName 

-- zadanie 5
select e.FirstName ,e.LastName, round(sum(od.Quantity*od.UnitPrice *(1- od.Discount)),2) as 'Wartosc zamowien' from Employees e 
inner join orders o on o.EmployeeID = e.EmployeeID and year(o.OrderDate) = '1997'
inner join [Order Details] od on od.OrderID  = o.OrderID 
group by e.EmployeeID ,e.FirstName ,e.LastName 
order by 'Wartosc zamowien' desc