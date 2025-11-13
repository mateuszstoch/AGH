-- zadanie 1
select e.FirstName ,e.LastName, round(sum(od.Quantity*od.UnitPrice *(1- od.Discount)),2) as 'Wartosc zamowien' from Employees e 
inner join orders o on o.EmployeeID = e.EmployeeID and year(o.OrderDate) = '1997'
inner join [Order Details] od on od.OrderID  = o.OrderID 
left join Employees e2 on e.EmployeeID = e2.ReportsTo 
where e2.EmployeeID is Null
group by e.EmployeeID ,e.FirstName ,e.LastName 

-- zadanie 2
select e.FirstName ,e.LastName, round(sum(od.Quantity*od.UnitPrice *(1- od.Discount))/count(distinct e2.EmployeeID),2) as 'Wartosc zamowien' 
from Employees e 
inner join orders o on o.EmployeeID = e.EmployeeID and year(o.OrderDate) = '1997'
inner join [Order Details] od on od.OrderID  = o.OrderID 
inner join Employees e2 on e.EmployeeID = e2.ReportsTo 
group by e.EmployeeID ,e.FirstName ,e.LastName